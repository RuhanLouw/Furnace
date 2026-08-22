# SQLite BOM Intro

SQLite is a small file-based database. For this project, the whole BOM database lives in one file:

```text
bom/furnace_bom.sqlite
```

You do not need a database server. The `sqlite3` command opens the file directly, runs SQL queries, and closes it again.

## Why Use SQLite Here

SQLite is useful for this BOM because the data is relational:

- one BOM item belongs to one assembly.
- one BOM item points to one canonical part.
- one part can have many supplier offers.
- one supplier can supply many parts.
- categories keep the list organized.

This is better than a flat spreadsheet once we want multiple suppliers per part, alternate parts, price history, and filtered views.

## Database Files

The BOM folder contains:

- `bom/furnace_bom.sqlite`: generated database file.
- `bom/schema.sql`: table and view definitions.
- `bom/seed_initial.sql`: starter BOM data.
- `bom/queries.sql`: useful example queries.
- `bom/README.md`: BOM-specific notes.

If the database ever gets damaged or needs to be regenerated, rebuild it from `schema.sql` and `seed_initial.sql`.

## Opening The Database

From the repo root:

```powershell
sqlite3 bom\furnace_bom.sqlite
```

Inside the SQLite prompt, useful setup commands are:

```sql
.headers on
.mode column
.tables
```

Exit with:

```sql
.quit
```

## One-Line Queries

You can also run queries directly from PowerShell without entering the SQLite prompt:

```powershell
sqlite3 bom\furnace_bom.sqlite ".headers on" ".mode column" "SELECT * FROM bom_flat;"
```

For CSV output:

```powershell
sqlite3 bom\furnace_bom.sqlite ".headers on" ".mode csv" "SELECT * FROM bom_requested_columns;"
```

## Main Tables

### `parts`

Canonical part records. This is where the actual part identity lives.

Useful fields:

- `part_id`
- `category_id`
- `manufacturer`
- `manufacturer_part_number`
- `part_name`
- `description`
- `selection_status`
- `notes`

### `supplier_offers`

Supplier-specific rows for a part. This is where links and prices live.

Useful fields:

- `offer_id`
- `part_id`
- `supplier_id`
- `supplier_sku`
- `price_amount`
- `currency`
- `link`
- `checked_at`
- `notes`

One part can have many rows in `supplier_offers`.

### `bom_items`

Numbered BOM rows connected to canonical parts.

Useful fields:

- `item_number`
- `subnumber`
- `part_id`
- `quantity`
- `use_case`
- `required_for`
- `design_notes`

The combination of `item_number` and `subnumber` gives the visible BOM numbering.

### `suppliers`

Supplier records such as Communica, RS South Africa, DIYElectronics, and Micro Robotics.

### `categories`

Logical categories such as Controller, Local HMI, Safety, Heater Control, Temperature Sensing, and Power.

## Main Views

### `bom_requested_columns`

This view gives exactly the requested columns:

```sql
SELECT * FROM bom_requested_columns;
```

Columns:

- `number`
- `subnumber`
- `category`
- `part`
- `use`
- `price`
- `link`

Because one part can have multiple supplier offers, the same BOM item can appear more than once with different supplier links.

### `bom_flat`

This view includes extra useful fields:

```sql
SELECT * FROM bom_flat;
```

Extra fields include:

- `currency`
- `supplier`
- `quantity`
- `unit`
- `selection_status`
- `checked_at`
- `supplier_notes`

Use this view for most engineering work.

## Useful Queries

Show the full BOM:

```sql
SELECT
    number,
    subnumber,
    category,
    part,
    use,
    price,
    currency,
    supplier,
    link
FROM bom_flat;
```

Show only HMI parts:

```sql
SELECT
    number,
    subnumber,
    part,
    price,
    supplier,
    link
FROM bom_flat
WHERE category = 'Local HMI';
```

Show parts without captured prices:

```sql
SELECT
    number,
    subnumber,
    category,
    part,
    supplier,
    link
FROM bom_flat
WHERE price IS NULL;
```

Show parts that still need final selection:

```sql
SELECT
    number,
    subnumber,
    category,
    part,
    selection_status,
    supplier_notes
FROM bom_flat
WHERE selection_status IN ('candidate', 'needs-selection');
```

Show all supplier offers for one part:

```sql
SELECT
    p.part_name,
    s.supplier_name,
    so.price_amount,
    so.currency,
    so.link,
    so.checked_at,
    so.notes
FROM supplier_offers so
JOIN parts p ON p.part_id = so.part_id
JOIN suppliers s ON s.supplier_id = so.supplier_id
WHERE p.part_name LIKE '%TFT%';
```

## Adding A Supplier Offer

First find the part and supplier IDs:

```sql
SELECT part_id, part_name FROM parts;
SELECT supplier_id, supplier_name FROM suppliers;
```

Then insert a supplier offer:

```sql
INSERT INTO supplier_offers (
    part_id,
    supplier_id,
    supplier_sku,
    price_amount,
    currency,
    price_basis,
    availability,
    link,
    checked_at,
    notes
) VALUES (
    2,
    3,
    'EXAMPLE-SKU',
    123.45,
    'ZAR',
    'listed inc VAT, verify before order',
    'in stock',
    'https://example.com/product',
    '2026-07-25',
    'Example supplier offer.'
);
```

## Adding A New Part

Add the canonical part first:

```sql
INSERT INTO parts (
    category_id,
    manufacturer,
    manufacturer_part_number,
    part_name,
    description,
    selection_status,
    notes
) VALUES (
    2,
    'Example Manufacturer',
    'EXAMPLE-MPN',
    'Example panel switch',
    'Example local HMI switch.',
    'candidate',
    'Added during design exploration.'
);
```

Then connect it to the BOM:

```sql
INSERT INTO bom_items (
    assembly_id,
    item_number,
    subnumber,
    part_id,
    quantity,
    unit,
    use_case,
    required_for,
    design_notes
) VALUES (
    1,
    1,
    7,
    last_insert_rowid(),
    1,
    'ea',
    'Example use case.',
    'prototype',
    'Example design note.'
);
```

## Exporting To CSV

Export the requested columns:

```powershell
sqlite3 bom\furnace_bom.sqlite ".headers on" ".mode csv" ".output bom\bom_requested_columns.csv" "SELECT * FROM bom_requested_columns;" ".output stdout"
```

Export the fuller engineering BOM:

```powershell
sqlite3 bom\furnace_bom.sqlite ".headers on" ".mode csv" ".output bom\bom_flat.csv" "SELECT * FROM bom_flat;" ".output stdout"
```

Current CSV exports are kept in:

- `bom/bom_requested_columns.csv`
- `bom/bom_flat.csv`

## Rebuilding The Database

Use this when changing `schema.sql` or `seed_initial.sql`:

```powershell
Remove-Item bom\furnace_bom.sqlite -ErrorAction SilentlyContinue
sqlite3 bom\furnace_bom.sqlite ".read bom/schema.sql"
sqlite3 bom\furnace_bom.sqlite ".read bom/seed_initial.sql"
```

Run a quick check:

```powershell
sqlite3 bom\furnace_bom.sqlite "PRAGMA integrity_check;"
```

Expected result:

```text
ok
```

## Editing Rule Of Thumb

For repeatable project history, prefer editing `bom/seed_initial.sql` and rebuilding the database. Direct edits to `furnace_bom.sqlite` are fine for quick experiments, but they are easier to lose unless the SQL change is also captured in the seed file.
