-- Migration 001: convert wpl table to InnoDB and add index on data_misurazione
--
-- Run this migration once on the production database.
-- It requires no data changes and is fully non-destructive.
--
-- Step 1: convert storage engine from MyISAM to InnoDB
ALTER TABLE `wpl` ENGINE = InnoDB;

-- Step 2: add index to speed up date-range queries used by rest.php and index.php
ALTER TABLE `wpl` ADD INDEX `idx_data_misurazione` (`data_misurazione`);
