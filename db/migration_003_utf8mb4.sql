-- Migration 003: convert table charset from latin1 to utf8mb4
-- utf8mb4 is the current MySQL standard and handles all Unicode characters correctly.
ALTER TABLE `wpl` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
