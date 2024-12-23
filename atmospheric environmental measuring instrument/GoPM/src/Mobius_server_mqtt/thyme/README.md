# Thyme

## Language Selection
[English](README.md) | [Korean](README_KR.md)

<br><br>

## Running Thyme

To start Thyme, use the following command:

```
node thyme
```

<br><br>

## Key Sections Explained

### Setting Server Address

```
conf.useprotocol = 'mqtt'; // select one for 'http' or 'mqtt' or 'coap' or 'ws'

// build cse
cse.host        = '114.71.220.59';
cse.port        = '2021';
```

- Select an appropriate protocol.
- Input the correct server address and port number.

<br>

### AE Creation
```
// build ae
ae.name         = 'gw';
```

- Define a name for the AE (Application Entity).
- The specified AE name will later be used in the format `Mobius/aeName`.

<br>

### CNT Creation

```
// build cnt
var count = 0;
cnt_arr[count] = {};
cnt_arr[count].parent = '/' + cse.name + '/' + ae.name;
cnt_arr[count++].name = 'motor';
cnt_arr[count] = {};
cnt_arr[count].parent = '/' + cse.name + '/' + ae.name;
cnt_arr[count++].name = 'data';
cnt_arr[count] = {};
cnt_arr[count].parent = '/' + cse.name + '/' + ae.name;
cnt_arr[count++].name = 'RPMData';
cnt_arr[count] = {};
cnt_arr[count].parent = '/' + cse.name + '/' + ae.name;
cnt_arr[count++].name = 'FanSpeed';
```

- Add as many CNTs (Containers) as you need, and name each one.
- These names will be used in the format `Mobius/aeName/cntName` or `Mobius/aeName/otherCntName/cntName`.

<br>

### Creating Subscriptions for CNT

```
sub_arr[count] = {};
sub_arr[count].parent = '/' + cse.name + '/' + ae.name + '/' + cnt_arr[0].name;
sub_arr[count].name = 'sub';
sub_arr[count++].nu = 'mqtt://' + cse.host + '/' + ae.id + '?ct=' + ae.bodytype; // mqtt
//sub_arr[count++].nu = 'http://' + ip.address() + ':' + ae.port + '/noti?ct=json'; // http
//sub_arr[count++].nu = 'Mobius/'+ae.name; // mqtt
// --------

// --------
sub_arr[count] = {};
sub_arr[count].parent = '/' + cse.name + '/' + ae.name + '/' + cnt_arr[2].name;
sub_arr[count].name = 'sub2';
//sub_arr[count++].nu = 'http://' + ip.address() + ':' + ae.port + '/noti?ct=json'; // http
//sub_arr[count++].nu = 'mqtt://' + cse.host + '/' + ae.id + '?rcn=9&ct=' + ae.bodytype; // mqtt
sub_arr[count++].nu = 'mqtt://' + cse.host + '/' + ae.id + '?ct=json'; // mqtt
// -------- */
```

- This section is for setting up MQTT subscriptions for specific CNTs.
- Use `cnt_arr[0].name` (or other CNTs as defined) to subscribe to the appropriate CNT.
- Multiple subscriptions can be set up if necessary.

> **Note:** When configuring MQTT subscriptions here, ensure that `conf.xml` is updated accordingly in the `tas` settings.
