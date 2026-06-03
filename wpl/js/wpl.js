var REFRESH_INTERVAL_MS = 10 * 60 * 1000; // 10 minutes

function parseData(misurazioni) {
    var measurements = [];
    misurazioni.forEach(function (element) {
        measurements.push([
            new Date(element.data_misurazione).getTime(),
            parseInt(element.distanza)
        ]);
    });
    return measurements;
}

function formatTimeSince(date) {
    var diffMs  = Date.now() - date.getTime();
    var diffMin = Math.floor(diffMs / 60000);
    var diffH   = Math.floor(diffMin / 60);
    var diffD   = Math.floor(diffH / 24);

    if (diffD > 0)  return diffD + ' day' + (diffD > 1 ? 's' : '') + ' ago';
    if (diffH > 0)  return diffH + ' hour' + (diffH > 1 ? 's' : '') + ' ago';
    if (diffMin > 0) return diffMin + ' minute' + (diffMin > 1 ? 's' : '') + ' ago';
    return 'just now';
}

function updateStatus(misurazioni) {
    var statusEl = document.getElementById('last-update-status');
    if (!statusEl) return;

    if (!misurazioni || misurazioni.length === 0) {
        statusEl.textContent = 'No data available.';
        statusEl.style.color = '#c0392b';
        return;
    }

    var latest     = misurazioni[0];
    var latestDate = new Date(latest.data_misurazione);
    var diffH      = (Date.now() - latestDate.getTime()) / 3600000;

    statusEl.textContent =
        'Last measurement: ' + latestDate.toLocaleString() +
        ' (' + formatTimeSince(latestDate) + ')';

    // Mark as stale if more than 30 hours old (sensor sends every 24 h)
    statusEl.style.color = diffH > 30 ? '#c0392b' : '#27ae60';
}
