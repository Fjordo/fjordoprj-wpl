-- Migration 004: add timestamp_arduino column to store NTP-synced time from Arduino Yun
-- The Arduino Yun's Linux SoC (AR9331) syncs time via NTP automatically.
-- This field is optional (NULL for rows inserted before this migration or by older firmware).
ALTER TABLE `wpl`
  ADD COLUMN `timestamp_arduino` datetime DEFAULT NULL
  COMMENT 'NTP-synced timestamp reported by the Arduino Yun Linux side';
