# Furnace BOM Database

The BOM is stored as a SQLite database with normalized tables and SQL seed files.

## Files

- `schema.sql` defines the database tables and views.
- `seed_initial.sql` seeds the first controller/HMI/safety/control BOM candidates.
- `queries.sql` contains useful read queries.
- `furnace_bom.sqlite` is the generated SQLite database.
- `bom_requested_columns.csv` is an Excel-friendly export of the requested columns.
- `bom_flat.csv` is an Excel-friendly export of the fuller engineering BOM view.
- `../docs/sqlite-bom-intro.md` explains SQLite basics and how to interact with this BOM.

## Table Model

- `categories`: logical part categories.
- `assemblies`: top-level assemblies or modules.
- `parts`: canonical part records.
- `suppliers`: supplier records.
- `supplier_offers`: supplier-specific price/link rows for each part.
- `bom_items`: numbered BOM items connected to canonical parts.

This keeps the BOM modular: one BOM item references one canonical part, and each part can have many supplier offers.

## Requested Columns

Use the `bom_requested_columns` view for the requested columns:

- `number`
- `subnumber`
- `category`
- `part`
- `use`
- `price`
- `link`

Use the `bom_flat` view when you also need supplier, currency, quantity, status, and checked date.

## CSV Exports

The CSV files are generated from database views:

- `bom_requested_columns.csv` comes from `bom_requested_columns`.
- `bom_flat.csv` comes from `bom_flat`.

Regenerate them from the repo root:

```powershell
sqlite3 bom\furnace_bom.sqlite ".headers on" ".mode csv" ".output bom\bom_requested_columns.csv" "SELECT * FROM bom_requested_columns;" ".output bom\bom_flat.csv" "SELECT * FROM bom_flat;" ".output stdout"
```

Because supplier offers are modular, one BOM item can appear on multiple CSV rows when it has multiple supplier options.

## Rebuild

From the repo root:

```powershell
Remove-Item bom\furnace_bom.sqlite -ErrorAction SilentlyContinue
sqlite3 bom\furnace_bom.sqlite ".read bom/schema.sql"
sqlite3 bom\furnace_bom.sqlite ".read bom/seed_initial.sql"
sqlite3 bom\furnace_bom.sqlite ".read bom/queries.sql"
```

## Caveats

Prices and stock are volatile. Every supplier row has a `checked_at` date and should be verified before ordering.

The current BOM is a candidate/prototype BOM. Heater-voltage, heater-current, thermocouple-type, enclosure, and safety-compliance decisions must be finalized before ordering high-power parts.
