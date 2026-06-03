# fjordoprj-wpl — Well Water Level Monitor

An IoT system that monitors the water level of a well using an Arduino Yun and an ultrasonic sensor (HC-SR04). Measurements are sent via WiFi to a PHP/MySQL backend and visualized on an interactive web dashboard.

Live dashboard: `https://fjordoprj.altervista.org/wpl/`

---

## Architecture

```text
Arduino Yun + HC-SR04
       |
       | HTTPS POST (every 24 h)
       v
  add.php  ──── INSERT ────> MySQL (wpl table)
                                   |
                          SELECT   |   SELECT
                        ┌──────────┴───────────┐
                        v                      v
                    rest.php              index.php
                  (JSON API)           (HTML table)
                        |
                        v
                   index.html
               (Highcharts dashboard)
```

---

## Hardware

| Component      | Details                                    |
|----------------|--------------------------------------------|
| Microcontroller| Arduino Yun (ATmega32U4 + AR9331 Linux SoC)|
| Sensor         | HC-SR04 ultrasonic distance sensor         |
| Optional       | 16x2 LCD display (for the LCD variant)     |

### Pin Connections — HC-SR04

| HC-SR04 Pin | Arduino Pin |
|-------------|-------------|
| VCC         | 5V          |
| GND         | GND         |
| Trig        | 8 (yellow)  |
| Echo        | 7 (green)   |

### Pin Connections — LCD (LCD variant only)

| LCD Pin | Arduino Pin |
|---------|-------------|
| RS      | 12          |
| Enable  | 11          |
| D4      | 5           |
| D5      | 4           |
| D6      | 3           |
| D7      | 2           |
| R/W     | GND         |
| VO      | 10kOhm wiper|

---

## Arduino Sketches

Two variants live in `wpl-sketches/`:

| Sketch                        | Display  | Upload interval | Well dims (dm) |
|-------------------------------|----------|-----------------|----------------|
| `Water_Level_WiFi_no_LCD.ino` | None     | 24 h            | h=30, d=20     |
| `Water_Level_WiFi.ino`        | 16x2 LCD | 1 h             | h=22, d=23     |

### Distance to Volume formula

```text
waterHeight (dm) = (sensor_distance_cm - 40) / 10
base (dm2)       = pi x (wellDiameter / 2)^2
volume (dm3)     = base x waterHeight
```

The -40 cm offset compensates for the sensor mounting position above the maximum water level.

### Measurement robustness

Each reading fires the HC-SR04 **5 times** and takes the **median** value, discarding spurious echoes. If the median falls outside the valid range (2-300 cm) the reading is treated as an error.

The `sendData()` function **retries up to 3 times** (with a 5-second pause between attempts) and uses the HTTP response code to detect failures. The onboard LED stays on if all retries fail.

### NTP timestamp

Both sketches call `getArduinoTimestamp()` before each POST. The function runs `date +%s` on the Yun's Linux SoC (AR9331), which syncs time via NTP automatically. The Unix timestamp is sent as the `ts` field and stored in the `timestamp_arduino` column alongside the server-side `NOW()` timestamp, making it easy to detect clock drift.

### API token

Before flashing, set `API_TOKEN` in the sketch to match the `API_TOKEN` value in `wpl/.env` on the server:

```cpp
const String API_TOKEN = "your_secret_token_here";
```

### WiFi Setup

The Arduino Yun's built-in WiFi must be configured before flashing. Follow the [official guide](https://www.twilio.com/blog/2015/02/arduino-wifi-getting-started-arduino-yun.html).

### Required Libraries

- `LiquidCrystal` (built-in)
- `Process` (part of the Bridge library, included with Arduino Yun support)

---

## Web Application (`wpl/`)

| File           | Role                                                       |
|----------------|------------------------------------------------------------|
| `add.php`      | POST endpoint — token auth, rate limiting, prepared INSERT |
| `rest.php`     | GET endpoint — returns data for selected time range        |
| `index.html`   | Highcharts dashboard (auto-refresh, dual series, range btn)|
| `index.php`    | Raw HTML table view (`?all=true` for all records)          |
| `connect.php`  | Shared MySQL connection helper (reads from environment)    |
| `js/wpl.js`    | Data parsers and last-update status helpers                |
| `css/wpl.css`  | Dashboard stylesheet                                       |
| `.env.example` | Template for required environment variables                |

### Environment variables

Copy `wpl/.env.example` to `wpl/.env` and fill in real values. The file is excluded from version control via `.gitignore`.

| Variable    | Description                              |
|-------------|------------------------------------------|
| `DB_HOST`   | MySQL host                               |
| `DB_USER`   | MySQL username                           |
| `DB_PASS`   | MySQL password                           |
| `DB_NAME`   | Database name                            |
| `API_TOKEN` | Shared secret for authenticating Arduino |

### Security

- All PHP files use `declare(strict_types=1)`.
- `add.php` validates the `API_TOKEN`, range-checks inputs, uses a prepared statement, and enforces a 5-minute rate limit.
- Both endpoints send `X-Content-Type-Options: nosniff` and `X-Frame-Options: DENY`.

### API

**POST** `/wpl/add.php`

| Parameter | Type   | Description                         |
|-----------|--------|-------------------------------------|
| `token`   | string | Must match `API_TOKEN` env variable |
| `dist`    | int    | Sensor distance in cm (0-500)       |
| `vol`     | float  | Computed volume in dm3              |
| `ts`      | int    | Unix timestamp from Arduino NTP     |

Returns `201 Created` on success, `403 Forbidden` on wrong token, `400 Bad Request` on invalid input, `429 Too Many Requests` if a measurement was stored in the last 5 minutes.

**GET** `/wpl/rest.php/my_fjordoprj/?range=<range>`

| `range` value | Data returned        |
|---------------|----------------------|
| `1m`          | Last 1 month         |
| `3m` (default)| Last 3 months        |
| `6m`          | Last 6 months        |
| `1y`          | Last 1 year          |
| `all`         | All records          |

Returns a JSON array:

```json
[
  {
    "distanza": 95,
    "volume_residuo": 314.16,
    "data_misurazione": "2021-09-01 08:00:00"
  }
]
```

---

## Database

```sql
CREATE TABLE `wpl` (
  `id`                int(11)  NOT NULL AUTO_INCREMENT,
  `distanza`          int(11)  NOT NULL DEFAULT 0,
  `volume_residuo`    float    DEFAULT NULL COMMENT 'volume residuo in dm3',
  `data_misurazione`  datetime DEFAULT CURRENT_TIMESTAMP,
  `timestamp_arduino` datetime DEFAULT NULL
                      COMMENT 'NTP-synced timestamp from Arduino Yun Linux',
  PRIMARY KEY (`id`),
  INDEX `idx_data_misurazione` (`data_misurazione`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

### Migrations

Run scripts in `db/` **in order** on the production database:

| File                                      | Description                              |
|-------------------------------------------|------------------------------------------|
| migration_001_innodb_and_index.sql        | Convert to InnoDB + add date index       |
| migration_002_fix_volume_unit_comment.sql | Fix column comment (dm3 not m3)          |
| migration_003_utf8mb4.sql                 | Convert charset to utf8mb4               |
| migration_004_arduino_timestamp.sql       | Add timestamp_arduino column             |

Database backups are stored in `db/`.

---

## Project Structure

```text
fjordoprj-wpl/
├── db/
│   ├── wpl_backup_2026-02-22.sql
│   ├── migration_001_innodb_and_index.sql
│   ├── migration_002_fix_volume_unit_comment.sql
│   ├── migration_003_utf8mb4.sql
│   └── migration_004_arduino_timestamp.sql
├── wpl/
│   ├── .env.example
│   ├── add.php
│   ├── rest.php
│   ├── index.html
│   ├── index.php
│   ├── connect.php
│   ├── css/
│   │   └── wpl.css
│   └── js/
│       └── wpl.js
└── wpl-sketches/
    ├── Water_Level_WiFi/
    │   └── Water_Level_WiFi.ino
    └── Water_Level_WiFi_no_LCD/
        └── Water_Level_WiFi_no_LCD.ino
```

---

## Frontend Dependencies (CDN)

- [Highcharts](https://www.highcharts.com/) (core + series-label + exporting + export-data)
