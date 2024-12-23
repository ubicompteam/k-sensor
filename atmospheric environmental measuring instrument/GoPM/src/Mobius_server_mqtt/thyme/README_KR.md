# thyme

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## thyme 구동 방법

```
node thyme
```

## 중요 부분 설명

### 서버 주소 넣는 부분

```
conf.useprotocol = 'mqtt'; // select one for 'http' or 'mqtt' or 'coap' or 'ws'

// build cse
cse.host        = '114.71.220.59';
cse.port        = '2021';
```

- 알맞는 프로토콜을 골라준뒤
- 알맞는 서버 주소와 포트 번호를 적어넣으세요

<br>

### AE 생성

```
// build ae
ae.name         = 'gw';
```

- 원하는 ae이름을 정해서 넣으세요
- 추후 Mobius/aeName 이런 형식으로 쓰입니다

<br>

### CNT 생성

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

- 원하는 CNT 갯수만큼 넣어주시고, 이름을 정해서 넣어주시면 됩니다.
- 추후 Mobius/aeName/cntName or Mobius/aeName/otherCntName/cntName  이런 형식으로 쓰입니다

<br>

### CNT 생성

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

- 원하는 cnt에 MQTT sub를 하는 부분입니다.
- `cnt_arr[0].name` 이부분에 위에서 정의한 순서를 지켜 원하는 곳에 sub를 하세요 (2개이상도 됩니다.)
- **이 작업을 할시 tas에서 conf.xml 작업을 해주셔야합니다.**
