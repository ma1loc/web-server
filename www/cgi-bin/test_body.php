#!/usr/bin/php-cgi

<?php
echo "Status: 200 OK\r\n";
echo "Content-Type: text/plain\r\n";
echo "\r\n";

echo "--- CGI Verification ---\n";
echo "Method: " . $_SERVER['REQUEST_METHOD'] . "\n";

if (!empty($_SERVER['QUERY_STRING'])) {
    echo "Query: " . $_SERVER['QUERY_STRING'] . "\n";
}

$body = file_get_contents('php://input');
$body_length = strlen($body);

echo "Body Length Received: " . $body_length . " bytes\n";
echo "Body Content: " . $body . "\n";
echo "------------------------\n";
?>

<!-- curl -POST http://localhost:8080/cgi-bin/test_body.php \
     --data-binary @large_body.txt \
     -H "Content-Type: text/plain" \
     -v -->