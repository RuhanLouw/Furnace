PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS categories (
    category_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    parent_category_id INTEGER REFERENCES categories(category_id),
    notes TEXT
);

CREATE TABLE IF NOT EXISTS assemblies (
    assembly_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    description TEXT
);

CREATE TABLE IF NOT EXISTS parts (
    part_id INTEGER PRIMARY KEY,
    category_id INTEGER NOT NULL REFERENCES categories(category_id),
    manufacturer TEXT,
    manufacturer_part_number TEXT,
    part_name TEXT NOT NULL,
    description TEXT,
    selection_status TEXT NOT NULL DEFAULT 'candidate',
    notes TEXT,
    UNIQUE (manufacturer, manufacturer_part_number)
);

CREATE TABLE IF NOT EXISTS suppliers (
    supplier_id INTEGER PRIMARY KEY,
    supplier_name TEXT NOT NULL UNIQUE,
    country TEXT,
    website TEXT,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS supplier_offers (
    offer_id INTEGER PRIMARY KEY,
    part_id INTEGER NOT NULL REFERENCES parts(part_id) ON DELETE CASCADE,
    supplier_id INTEGER NOT NULL REFERENCES suppliers(supplier_id),
    supplier_sku TEXT,
    price_amount NUMERIC,
    currency TEXT NOT NULL DEFAULT 'ZAR',
    price_basis TEXT,
    availability TEXT,
    link TEXT NOT NULL,
    checked_at TEXT NOT NULL,
    notes TEXT,
    UNIQUE (part_id, supplier_id, supplier_sku, link)
);

CREATE TABLE IF NOT EXISTS bom_items (
    bom_item_id INTEGER PRIMARY KEY,
    assembly_id INTEGER NOT NULL REFERENCES assemblies(assembly_id),
    parent_bom_item_id INTEGER REFERENCES bom_items(bom_item_id),
    item_number INTEGER NOT NULL,
    subnumber INTEGER NOT NULL DEFAULT 0,
    part_id INTEGER NOT NULL REFERENCES parts(part_id),
    quantity NUMERIC NOT NULL DEFAULT 1,
    unit TEXT NOT NULL DEFAULT 'ea',
    use_case TEXT NOT NULL,
    required_for TEXT NOT NULL DEFAULT 'prototype',
    design_notes TEXT,
    UNIQUE (assembly_id, item_number, subnumber)
);

CREATE INDEX IF NOT EXISTS idx_bom_items_part_id ON bom_items(part_id);
CREATE INDEX IF NOT EXISTS idx_supplier_offers_part_id ON supplier_offers(part_id);
CREATE INDEX IF NOT EXISTS idx_supplier_offers_supplier_id ON supplier_offers(supplier_id);

CREATE VIEW IF NOT EXISTS bom_flat AS
SELECT
    bi.item_number AS number,
    bi.subnumber AS subnumber,
    c.name AS category,
    p.part_name AS part,
    bi.use_case AS use,
    so.price_amount AS price,
    so.currency AS currency,
    s.supplier_name AS supplier,
    so.link AS link,
    bi.quantity AS quantity,
    bi.unit AS unit,
    p.selection_status AS selection_status,
    so.checked_at AS checked_at,
    so.notes AS supplier_notes
FROM bom_items bi
JOIN parts p ON p.part_id = bi.part_id
JOIN categories c ON c.category_id = p.category_id
LEFT JOIN supplier_offers so ON so.part_id = p.part_id
LEFT JOIN suppliers s ON s.supplier_id = so.supplier_id
ORDER BY bi.item_number, bi.subnumber, s.supplier_name;

CREATE VIEW IF NOT EXISTS bom_requested_columns AS
SELECT
    number,
    subnumber,
    category,
    part,
    use,
    price,
    link
FROM bom_flat;
