<?php
mysqli_report(MYSQLI_REPORT_OFF);
$servername = "localhost";
$username = "esp_user";
$password = "CHANGE_THIS_DATABASE_PASSWORD";
$dbname = "esp_db";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    http_response_code(500);
    exit("Database connection failed. Check database credentials.");
}
$conn->set_charset("utf8mb4");
$result = $conn->query("SELECT id, distance, status, created_at FROM sensor_logs ORDER BY id DESC LIMIT 200");
if (!$result) {
    http_response_code(500);
    exit("Could not load sensor records.");
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Distance Monitoring Dashboard</title>
  <meta http-equiv="refresh" content="3">
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body class="bg-light">
  <main class="container py-5">
    <div class="text-center mb-4">
      <h2>ESP32 Distance Monitoring Dashboard</h2>
      <p class="text-muted">Sensor readings from MySQL · refreshes every 3 seconds</p>
    </div>
    <div class="card shadow-sm">
      <div class="card-body">
        <div class="table-responsive">
          <table class="table table-striped table-hover align-middle mb-0">
            <thead class="table-dark">
              <tr><th>ID</th><th>Distance</th><th>Status</th><th>Timestamp</th></tr>
            </thead>
            <tbody>
            <?php if ($result->num_rows > 0): ?>
              <?php while ($row = $result->fetch_assoc()): ?>
                <?php $isAlert = strpos($row["status"], "ALERT") !== false; ?>
                <tr>
                  <td><?= (int)$row["id"] ?></td>
                  <td><strong><?= htmlspecialchars((string)$row["distance"], ENT_QUOTES, "UTF-8") ?></strong> cm</td>
                  <td><span class="badge <?= $isAlert ? "bg-danger" : "bg-success" ?>"><?= htmlspecialchars($row["status"], ENT_QUOTES, "UTF-8") ?></span></td>
                  <td><?= htmlspecialchars($row["created_at"], ENT_QUOTES, "UTF-8") ?></td>
                </tr>
              <?php endwhile; ?>
            <?php else: ?>
              <tr><td colspan="4" class="text-center text-muted">No sensor records found yet.</td></tr>
            <?php endif; ?>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </main>
</body>
</html>
<?php
$result->free();
$conn->close();
?>