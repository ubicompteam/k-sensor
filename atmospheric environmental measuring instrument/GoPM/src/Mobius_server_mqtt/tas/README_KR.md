# tas

## 언어 선택

[English](README.md) | [한국어](README_KR.md)

<br><br>

## 코드 설명
- app.js : 이 코드는 Node.js 스크립트로, 설정 파일을 읽고 Python TCP 서버와 통신을 관리하며 연결 상태를 유지하기 위한 감시(watchdog) 메커니즘을 포함하고 있음
           설정은 XML 파일(`conf.xml`)에서 읽어옴
- conf.xml : `conf.xml`은 시스템의 부모 서버 정보와 데이터 다운로드 설정을 정의하는 XML 파일임, mqtt설정시 꼭 설정해줘야 함

<br><br>

## app.js 중요 코드만 설명

### 이 함수는 **`comm_num`** 파라미터를 받아 Python 서버와 통신하여 LED를 제어함

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

1. **Python TCP 서버에 연결**: `net.Socket()`을 사용하여 로컬 호스트의 12345 포트에 연결하고, 데이터를 보낸 후 연결을 종료함
2. **오류 처리**: 서버 연결 실패 시 오류 메시지를 출력함
3. **연결 상태 확인**: `status`가 1일 때만 Python 서버와의 연결을 시도함

## conf.xml

- `conf.xml` 파일은 시스템의 주요 설정을 정의하여 서버 연결 정보와 데이터 다운로드 옵션을 지정합니다. 특히, **mqtt** 설정 시 중요한 역할을 합니다.

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

### XML 구조 개요

- **루트 요소**: `<m2m:conf>`
  - `m2m:conf` 네임스페이스에서 주요 설정이 정의되며, 이는 oneM2M 표준 프로토콜을 따릅니다.
  - TAS(Thing Adaptation Software)와 다운로드할 데이터 채널에 대한 설정을 포함하는 하위 요소를 갖고 있습니다.

### XML 요소 상세 설명

1. **`<tas>` 요소**
   - TAS(Thing Adaptation Software) 설정을 나타내며, 기기 간의 통신을 관리합니다.
   - **`<parenthostname>`**: 상위 서버 호스트명을 정의합니다. 여기서는 `localhost`로 설정되어 있으며, 이는 로컬 네트워크에서 통신이 이루어짐을 의미합니다.
   - **`<parentport>`**: 상위 서버의 포트 번호를 정의합니다. 예시에서는 `3105`번 포트가 사용됩니다.

2. **`<download>` 요소**
   - 다운로드할 데이터 채널을 정의하며, 동일한 형식의 `<download>` 요소가 여러 개 있을 수 있습니다.
   - **`<ctname>`**: 다운로드할 컨텐츠의 이름을 지정합니다. 예시에서는 `motor`와 `RPMData`가 설정되어 있습니다.
   - **`<id>`**: 각 다운로드 채널의 고유 ID를 지정합니다. 예시에서는 `motor` 채널의 ID가 `mb`, `RPMData` 채널의 ID가 `rpm`으로 설정되어 있습니다.

### 전체 설정 요약

- `parenthostname` 및 `parentport` 값을 통해 상위 서버에 연결하여 데이터를 송수신합니다.
- 각 `<download>` 요소는 시스템이 데이터를 수신할 채널을 정의하며, 각 채널의 `ctname`과 `id` 값을 통해 데이터를 식별합니다.

이러한 설정을 통해 시스템은 필요한 데이터 채널을 지정된 서버와 통신할 수 있습니다.


