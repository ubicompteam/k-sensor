# tas

## Language Selection

[English](README.md) | [한국어](README_KR.md)

<br><br>

## Code Overview
- **app.js**: This is a Node.js script that reads configuration files, manages communication with a Python TCP server, and includes a watchdog mechanism to maintain connection stability. Configuration settings are loaded from an XML file (`conf.xml`).
- **conf.xml**: This XML file defines server information and data download settings, which must be configured, especially for MQTT use.

<br><br>

## Key Code Explanations in app.js

### This function receives the **`comm_num`** parameter to communicate with the Python server and control an LED.

```
function control_led(comm_num) {
    const inputData = comm_num;

    // Indicating that the control_led function has been called
    console.log(`control_led called with data: ${inputData}`);

    if (status === 1) {
        // Connect to Python TCP server
        const client = new net.Socket();
        client.connect(12345, 'localhost', function () {
            console.log('Connected to Python server');
            client.write(inputData + '\n', () => {
                // 데이터를 보낸 후 연결 종료
                client.end();
            });
        });

        client.on('data', function (data) {
            console.log('Received: ' + data);
        });

        client.on('close', function () {
            console.log('Connection closed');
        });

        client.on('error', function (err) {
            console.error('Failed to connect to Python server:', err);
        });
    } else {
        console.log('Status is not 1, skipping Python script execution');
    }
}
```

1. Connection to the Python TCP Server: Uses net.Socket() to connect to the localhost on port 12345, sends data, then closes the connection.
2. Error Handling: Logs an error message if the server connection fails.
3. Connection Status Check: Attempts to connect to the Python server only if status is set to 1.

## conf.xml

- The `conf.xml` file defines the key configurations of the system, specifying server connection details and data download options. This file is especially important for **mqtt** setup.

```
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<m2m:conf xmlns:m2m="http://www.onem2m.org/xml/protocols" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <tas>
        <parenthostname>localhost</parenthostname>
        <parentport>3105</parentport>
    </tas>
    <download>
        <ctname>motor</ctname>
        <id>mb</id>
    </download>
    <download>
        <ctname>RPMData</ctname>
        <id>rpm</id>
    </download>
</m2m:conf>

```

### XML Structure Overview

- **Root Element**: `<m2m:conf>`
  - This element, using the `m2m:conf` namespace, defines the primary configurations according to the oneM2M protocol standard.
  - It includes sub-elements for TAS (Thing Adaptation Software) and the data channels to download.

### XML Element Detailed Explanation

1. **`<tas>` Element**
   - Defines the TAS (Thing Adaptation Software) configuration, which manages communication between devices.
   - **`<parenthostname>`**: Specifies the hostname of the parent server, set to `localhost` here, indicating that communication occurs within a local network.
   - **`<parentport>`**: Defines the port number for the parent server. In this example, port `3105` is used.

2. **`<download>` Element**
   - Defines each data channel to be downloaded. Multiple `<download>` elements can be included.
   - **`<ctname>`**: Specifies the name of the content to download. In this example, `motor` and `RPMData` are configured.
   - **`<id>`**: Provides a unique ID for each download channel, with `motor` channel’s ID as `mb` and `RPMData` channel’s ID as `rpm`.

### Summary of Settings

- The `parenthostname` and `parentport` values enable data communication with the parent server.
- Each `<download>` element defines a data channel for the system to receive, identifying data with specific `ctname` and `id` values.

These settings allow the system to communicate with the designated server for the necessary data channels.
