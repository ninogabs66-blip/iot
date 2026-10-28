<?php
mysqli_report(MYSQLI_REPORT_OFF);
$servername = "localhost";
$username = "esp_user";
$password = "CHANGE_THIS_DATABASE_PASSWORD";
$dbname = "esp_db";

if ($_SERVER["REQUEST_METHOD"] !== "POST") {
    http_response_code(405);
    header("Allow: POST");
    exit("POST requests only.");
}

if (!isset($_POST["distance"], $_POST["status"]) ||
    !is_numeric($_POST["distance"]) ||
    trim((string)$_POST["status"]) === "") {
    http_response_code(400);
    exit("Required form fields: distance and status.");
}

$distance = (float)$_POST["distance"];
$status = trim((string)$_POST["status"]);
if ($distance < 0 || $distance > 400 || strlen($status) > 100) {
    http_response_code(400);
    exit("Invalid distance or status length.");
}

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    http_response_code(500);
    exit("Database connection failed.");
}
$conn->set_charset("utf8mb4");

$stmt = $conn->prepare("INSERT INTO sensor_logs (distance, status) VALUES (?, ?)");
if (!$stmt) {
    http_response_code(500);
    exit("Could not prepare database insert.");
}
$stmt->bind_param("ds", $distance, $status);
if ($stmt->execute()) {
    echo "Record inserted successfully";
} else {
    http_response_code(500);
    echo "Failed to insert record.";
}
$stmt->close();
$conn->close();
?>