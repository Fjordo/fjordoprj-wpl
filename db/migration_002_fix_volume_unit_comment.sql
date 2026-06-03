-- Migration 002: correct the COMMENT on volume_residuo (value is dm³, not m³)
ALTER TABLE `wpl`
  MODIFY COLUMN `volume_residuo` float DEFAULT NULL COMMENT 'volume residuo in dm³';
