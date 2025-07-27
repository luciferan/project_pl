# project_pl
- 개인 기록용 프로젝트
- 여기저기 분산된 코드를 모아서 관리하는게 목표


history
# todo - cpp
- 로그 남기는 위치도 변경. 소스파일 위치에 남고 있었네
- 패킷큐 구조 변경. 처리는 커맨드큐로 이동할수 있도록 수정
- enum 전부 enum class로 변경
- 네이밍 정리 필요함. 예전 스타일이랑 aa스타일이랑 섞여버림

- 당장은 시간이 없어서 못하겠지만 추가할 내용 정리. 잊어버리기전에
- 서버와 클라의 연결 소켓 추가
- web소켓 추가
- sql 연결 추가
- sql agent 서버 추가
- 서버간 연결
- 동기화서버 추가. 클라-동기화, 클라-서버, 동기화-서버의 트라이앵글 연결
- 모니터링 추가
- c#으로 포팅
- rapidjson이 utf-8 인코딩만 읽을수 있는 문제가 있어서 다른거로 바꿔야할까싶음
- 동기화서버는 rpc로 작성
- grpc, quic 확인
- msquic 테스트코드를 본 코드에 통합. 에코테스트까지
- asio 테스트 코드 작성. 에코테스트 이후에 적용할지 고민

# todo - cs
- 공용부분을 분리
- 내가 추가하지 않은 using이 자꾸 늘어나는데 vs에서 자동 추가되는건지 코파일럿이 추가하는건지 모르겠다

#20250727 - cs
- Server_v2로 구조 변경중
- auth, world, db로 분리
- json config 추가
- 패킷 데이터에 대한 내용은 _Network에서 _Common 으로 이동
- 네트워크 코드를 Network_v2로 공용 코드 분리

#20250630 - cpp
- msquic, openssl 라이브러리 정적 링크용으로 빌드해서 추가
- msquic 테스트 코드 svr_quic, cli_quic 추가
- 연결, 빈데이터 송수신까지만 확인함

#20250614 - cs
- PacketHandler의 인터페이스 IPacketHandler를 추가
- packet, Serializer, Handler를 PL_Network로 이동

#20250608 - cs
- 작업내용 중간 커밋
- project_pl_cpp_20과 같이 쓰기 위해 변경중
- packet head+body+tail 구조로 변경
- pl_cpp서버에 에코 테스트
- pl_cpp 패킷 추가
- pl_cpp에서 사용한 방식의 packetSerializer 추가
- PacketHandlerQueue를 LinkedList에서 Queue로 변경
- mysql 연결 테스트

#20250608 - cpp
- project_pl_cs_9와 같이 쓰기 위해 변경
- packet에 사용하는 type을 정의타입으로 변경 (INT16, INT32...)
- header {check, length} - body {packetType, data} - tail {check, seq, time}

#20250601 - cs
- PacketHandler를 TaskScheduler로 변경
- PacketParser에서 파싱되는 즉시 PacketHandler에 밀어넣고 PacketHandler에서 Task로 생성해서 처리

#20250601 - cpp
- packet serializer 추가
- serialize로 send 하도록 변경
- PacketFunctor 정리

#20250515 - cs
- recv받은 패킷을 그대로 send. 에코 처리 확인
- PacketHandler를 class PacketHandler로 이동
- PacketParser.DataParsing에 성공하면 PacketHandler.PacketProcess를 호출하도록 변경
- PacketHandler는 함수포인터..가 아니라 딕셔너리에 PacketType과 Func<>로 리스트업해서 호출되도록 변경
- 패킷 처리부분을 async task로 작성. 일반적인 동작은 되는듯하나 비동기 작동에 대한 테스트는 추후에 확인
- async task를 사용한 영향으로 PlayerSession까지 줄줄이 async task로 변경됨

#20250513 - cs
- pending체크 잘못되서 recv못받던거 수정
- packet recv받는 부분까지 확인

#20250512
- c# 기본 구조 작성중

#20250511
- rapidjson 라이브러리를 사용해서 ConfigLoader 추가
- vs .editorconfig 추가

#20250505
- lexicon 작성중
- 일부 파일명 변경, 일부 클래스명 변경
- eNetworkBuffer를 class NetworkBuffer에서 struct OverlappedEx로 이동. 이름변경
- AcceptEx 적용
- FileLog 큐에 모았다가 쓰게 변경

#20250323
- 새로운 프로젝트를 시작해서 여기까지를 기본코드로 하고 새로운 저장소로 이동
- 변경사항은 지속적으로 정리해서 업데이트

#20250322
- 퇴장처리 안되는 버그 수정
- 하트비트 꺼놓은거 복구

#20250322
- enum class로의 변경 진행중
- packethandler추가. 패킷처리를 함수포인터로 변경
- commandunitbase 수정
- broadcast 오류 수정
- lock을 mutex로 바꿔서 발생한 문제들 수정
- 코드 정리

#20250310
- winsockapi 충돌이 너무 심해서 프리컴파일드헤더 추가
- enum 일부를 enum class로 변경
- connector와 userSession 관계 정리중
- PostQueuedCompletionStatus 제거
- ObjectPoolMgrBase를 사용하던 클래스 일부를 ObjectMgrBase로 변경
- 동기화를 mutex로 변경
- svr에 CommandUnitQueue 추가
- svr 패킷 처리방식 CommandUnitQueue로 변경
- usersession리스트를 zone객체에 넣어서 관리. zone객체 기반으로 broadcast
- 일부 클래스 네이밍 변경

#20250306
- 메모리풀 변경
- std::allocator를 사용하는 ObjectPool
- ObjectPool을 사용하면서 사용중인 객체가 관리가 필요하면 ObjectMgrBase
- new 할당으로 처리하는 ObjectPoolMgrBase

#20250226
- command queue 추가
- 패킷 프로토콜 수정
- 데이터가 잘못 전송되고 있던걸 알게되서 수정
- 사소한 formatting 수정
- 파일 네이밍 수정

#20250225
- std::thread로 통일
- 사소한 formatting 수정

#20250223
- MySQL db 연결 클래스 추가
- connector-c++를 사용하는 클래스 추가
- connector-c++가 꽤 좋아서 c api를 사용하는 클래스는 작업 중단
- 나중에 필요한 상황이되면 다시 제작하거나 code_rubble쪽에 추가하는것으로

#20250218
- MySQL 연결테스트중
- MySQL-connector를 사용하는 부분, MySQL C API를 사용하는 부분 둘다 확인
- MySQL 라이브러리가 추가되서 용량이 커짐

#20250216
- FormatA, FormatW 버퍼를 힙으로 이동
- GetFileList를 <filesystem> 으로 변경
- PacketLog snwprintf에서 std::format으로 수정 적용
- 오류때문에 주석처리해놓은 Log 되살림
- CTimeSet _get_timezone 적용으로 변경
- CPerformanceChecker를 chrono::steady_clock으로 변경

#20250202
- 시간이 없어서 작업에 텀이 길어진 이유로 중간커밋
- 자주 수정안하는 소스를 /_lib 폴더로 이동
- 메모리풀에 객체를 쓰고싶은데 락객체 복사문제가 해결되지 않아서 포인터로
- std::jthread의 종료 시점이 제어가 안되서 std::thread에 stop_token을 추가해서 사용
- _snprintf_s를 다 빼고 std::format를 쓰고싶었으나 바이너리와 포인터 포메팅관련 문제가 조금 있어서 _snprintf_s를 계속 써야할듯

#20250114
- std::jthread를 사용하는것이 그다지 효용성이 높아 보이지 않음
- windows에서는 _beginthreadex를 쓰는것이 가장 효율적으로 보이나 그럴꺼면 mutex도 쓰지말고 CRITICAL_SECTION을 쓰는게 좋을것같음
- 일단 std::thread를 사용하는 방향으로 진행

#20241230
- CRITICAL_SECTION을 직접 사용하는 부분을 모두 제거
- 락 객체를 가지는 class Lock 으로 변경
- Lock객체를 받아서 사용하는 class SafeLock으로 통일
- mutex로 변경하려 했으나 소유자의 복제 문제가 있어서 일단 보류

#20241227
- 프로젝트명을 project_pl_cpp_20로 변경
- c++20 스타일로 변경계획

#20241226
- 로컬 저장소가 터진 이유로 github로 이동

#20241118
- /std:c++latest로 변경

#20241115
- /std:c++20으로 마이그레이션
- class network 코드 일부 정리

#20241114
- vs2015에서 제작된 project_litter/project_cpp_n을 베이스로 시작
- 이 코드를 vs2022로 마이그레이션
