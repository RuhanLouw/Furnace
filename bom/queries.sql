.headers on
.mode column

-- Full BOM with one row per supplier offer.
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

-- The exact requested columns.
SELECT
    number,
    subnumber,
    category,
    part,
    use,
    price,
    link
FROM bom_requested_columns;

-- Parts that still need final selection or confirmed price.
SELECT
    number,
    subnumber,
    category,
    part,
    selection_status,
    supplier,
    price,
    supplier_notes
FROM bom_flat
WHERE selection_status IN ('candidate', 'needs-selection')
   OR price IS NULL;
