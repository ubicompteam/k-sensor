const net = require('net');
const util = require('util');
const fs = require('fs');
const xml2js = require('xml2js');

const wdt = require('./wdt');

let useparentport = '';
let useparenthostname = '';

let upload_arr = [];
let download_arr = [];

let conf = {};

let status = 0; // Status is initially set to 0

// This is an async file read
fs.readFile('conf.xml', 'utf-8', function (err, data) {
    if (err) {
        console.log("FATAL An error occurred trying to read in the file: " + err);
        console.log("error : set to default for configuration");
    } else {
        const parser = new xml2js.Parser({ explicitArray: false });
        parser.parseString(data, function (err, result) {
            if (err) {
                console.log("Parsing An error occurred trying to read in the file: " + err);
                console.log("error : set to default for configuration");
            } else {
                const jsonString = JSON.stringify(result);
                conf = JSON.parse(jsonString)['m2m:conf'];

                useparenthostname = conf.tas.parenthostname;
                useparentport = conf.tas.parentport;

                if (conf.upload != null) {
                    if (conf.upload['ctname'] != null) {
                        upload_arr[0] = conf.upload;
                    } else {
                        upload_arr = conf.upload;
                    }
                }

                if (conf.download != null) {
                    if (conf.download['ctname'] != null) {
                        download_arr[0] = conf.download;
                    } else {
                        download_arr = conf.download;
                    }
                }

                // Set status to 1 after configuration is loaded
                status = 1;
            }
        });
    }
});

let tas_state = 'init';
let upload_client = null;
let tas_download_count = 0;

function on_receive(data) {
    //console.log("come");
    //console.log(tas_state + '---------------------12345');
    if (tas_state === 'connect' || tas_state === 'reconnect' || tas_state === 'upload') {
        const data_arr = data.toString().split('<EOF>');
        if (data_arr.length >= 2) {
            for (let i = 0; i < data_arr.length - 1; i++) {
                const line = data_arr[i];
                const sink_str = util.format('%s', line.toString());
                const sink_obj = JSON.parse(sink_str);

                if (sink_obj.ctname == null || sink_obj.con == null) {
                    console.log('Received: data format mismatch');
                } else {
                    if (sink_obj.con === 'hello') {
                        console.log('Received: ' + line);

                        if (++tas_download_count >= download_arr.length) {
                            tas_state = 'upload';
                        }
                    } else {
                        for (let j = 0; j < upload_arr.length; j++) {
                            if (upload_arr[j].ctname === sink_obj.ctname) {
                                console.log('ACK : ' + line + ' <----');
                                break;
                            }
                        }
			 console.log(download_arr + '-----------------------');
                        for (let j = 0; j < download_arr.length; j++) {
                            //console.log(download_arr + '---------------------');
                            if (download_arr[j].ctname === sink_obj.ctname) {
                                const g_down_buf = JSON.stringify({ id: download_arr[j].ctname, con: sink_obj.con });
                                console.log(g_down_buf + ' <----');
                                control_led(sink_obj.con);
                                break;
                            }
                            //console.log("come");
                        }
                    }
                }
            }
        }
    }
}

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


function tas_watchdog() {
    if (tas_state === 'init') {
        upload_client = new net.Socket();

        upload_client.on('data', on_receive);

        upload_client.on('error', function (err) {
            console.log(err);
            tas_state = 'reconnect';
        });

        upload_client.on('close', function () {
            console.log('Connection closed');
            upload_client.destroy();
            tas_state = 'reconnect';
        });

        if (upload_client) {
            console.log('tas init ok');
            tas_state = 'init_thing';
        }
    } else if (tas_state === 'init_thing') {
        // init things
        control_led('0');

        tas_state = 'connect';
    } else if (tas_state === 'connect' || tas_state === 'reconnect') {
        upload_client.connect(useparentport, useparenthostname, function () {
            console.log('upload Connected');
            tas_download_count = 0;
            for (let i = 0; i < download_arr.length; i++) {
                console.log('download Connected - ' + download_arr[i].ctname + ' hello');
                const cin = { ctname: download_arr[i].ctname, con: 'hello' };
                upload_client.write(JSON.stringify(cin) + '<EOF>');
            }

            if (tas_download_count >= download_arr.length) {
                tas_state = 'upload';
            }
        });
    }
}

// Every 3 seconds, check if the TAS is not working
wdt.set_wdt(require('shortid').generate(), 3, tas_watchdog);

