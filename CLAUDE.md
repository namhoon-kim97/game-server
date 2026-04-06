# IOCP 게임서버 프로젝트

## 프로젝트 개요
- **목적**: 1-2년차 게임서버 개발자 수준 포트폴리오. 추후 1인 로그라이크 게임 출시 시 실제 서버로 활용 예정
- **개발자**: 현재 C++ 게임서버 5개월차, 공부 겸 진행 중
- **장르**: 싱글 플레이어 로그라이크 + 랭킹/세이브

## 기술 스택
- 언어: C++17
- 플랫폼: Windows (WSL2 환경에서 개발, 빌드는 Visual Studio)
- 네트워크: IOCP (I/O Completion Port) + TCP only
- DB: SQLite (sqlite3 amalgamation, vendor 폴더에 직접 포함)
- JSON: nlohmann/json (single header, vendor 폴더에 직접 포함)
- 빌드: Visual Studio Solution (.sln)

## 작업 방식
- **잘게 쪼개서** 하나씩 진행 (공부 겸이라 각 단계를 이해하며 진행)
- 각 작업마다 왜 이렇게 설계하는지 설명 포함

---

## VS 솔루션 구성 (3개 프로젝트)

| 프로젝트 | 타입 | 역할 |
|---|---|---|
| `Shared` | Static Library (.lib) | 락/메모리풀/로거/타이머/패킷 정의 — 공통 인프라 |
| `Server` | Console Application | IOCP 코어/네트워크/게임 로직/DB |
| `ClientStub` | Console Application | 테스트용 클라이언트 시뮬레이터 |

### VS 빌드 설정 (3개 프로젝트 모두)
- `/std:c++17`, `/W4`, `/WX` (경고를 오류로)
- 전처리기: `_WIN32_WINNT=0x0601;NOMINMAX;WIN32_LEAN_AND_MEAN`
- Server 추가: `SQLITE_THREADSAFE=2`
- Server, ClientStub → 참조 추가 → Shared 체크

---

## 폴더 구조

```
game-server/
├── game-server.sln
├── CLAUDE.md                    ← 이 파일
├── docs/
│   ├── architecture.md
│   ├── packet_spec.md
│   └── db_schema.sql
├── Shared/                      ← Static Library
│   ├── Core/
│   │   ├── SpinLock.h
│   │   ├── RWLock.h
│   │   ├── LockGuard.h
│   │   ├── MemoryPool.h/.cpp
│   │   ├── ObjectPool.h
│   │   ├── Logger.h/.cpp
│   │   └── Timer.h/.cpp
│   └── Protocol/
│       ├── PacketHeader.h
│       ├── PacketTypes.h
│       └── Packets.h
├── Server/                      ← Console App
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Core/
│   │   │   ├── IOCPObject.h
│   │   │   ├── IOCPCore.h/.cpp
│   │   │   └── WorkerThread.h/.cpp
│   │   ├── Network/
│   │   │   ├── Listener.h/.cpp
│   │   │   ├── Session.h/.cpp
│   │   │   ├── SessionManager.h/.cpp
│   │   │   ├── RecvBuffer.h/.cpp
│   │   │   └── SendBuffer.h/.cpp
│   │   ├── Packet/
│   │   │   ├── PacketHandler.h/.cpp
│   │   │   └── PacketDispatcher.h/.cpp
│   │   ├── Auth/
│   │   │   ├── AuthHandler.h/.cpp
│   │   │   └── TokenManager.h/.cpp
│   │   ├── Game/
│   │   │   ├── GameService.h/.cpp
│   │   │   ├── GameValidator.h/.cpp
│   │   │   └── RankingService.h/.cpp
│   │   └── DB/
│   │       ├── DBConnection.h/.cpp
│   │       ├── DBConnectionPool.h/.cpp
│   │       ├── DBTaskQueue.h/.cpp
│   │       └── Repository/
│   │           ├── IAccountRepository.h
│   │           ├── AccountRepository.h/.cpp
│   │           ├── IGameRepository.h
│   │           ├── GameRepository.h/.cpp
│   │           └── RankingRepository.h/.cpp
│   └── vendor/
│       ├── sqlite/              ← sqlite3.h + sqlite3.c
│       └── nlohmann/            ← json.hpp
└── ClientStub/                  ← Console App (테스트용)
    └── src/
        ├── main.cpp
        └── TestScenarios.h/.cpp
```

---

## 전체 작업 목록 (26개)

### Phase 1: Foundation (기반 인프라)
- [x] **1-1** 폴더 구조 생성 + VS 솔루션 설정 안내
- [ ] **1-2** SpinLock 구현 (`Shared/Core/SpinLock.h`)
- [ ] **1-3** RWLock + LockGuard 구현 (`Shared/Core/`)
- [ ] **1-4** Logger 구현 비동기 (`Shared/Core/Logger.h/.cpp`)
- [ ] **1-5** MemoryPool 구현 (`Shared/Core/MemoryPool.h/.cpp`)
- [ ] **1-6** ObjectPool 구현 (`Shared/Core/ObjectPool.h`)
- [ ] **1-7** PacketHeader + PacketTypes 정의 (`Shared/Protocol/`)
- [ ] **1-8** PacketWriter + PacketReader 구현 (`Shared/Protocol/Packets.h`)

### Phase 2: IOCP Core
- [ ] **2-1** IOCPObject + IOCPEvent 기반 클래스
- [ ] **2-2** IOCPCore 구현
- [ ] **2-3** WorkerThread 구현
- [ ] **2-4** RecvBuffer 구현 (링 버퍼)
- [ ] **2-5** SendBuffer 구현
- [ ] **2-6** Listener 구현 (AcceptEx)

### Phase 3: Session Layer
- [ ] **3-1** Session 구현
- [ ] **3-2** SessionManager 구현
- [ ] **3-3** PacketDispatcher 구현 + echo 테스트

### Phase 4: DB Layer
- [ ] **4-1** DBConnection 구현 (SQLite 래퍼)
- [ ] **4-2** DBConnectionPool + DBTaskQueue 구현
- [ ] **4-3** Repository 구현 (Account/Game/Ranking)

### Phase 5: Game Systems
- [ ] **5-1** AuthHandler + TokenManager 구현
- [ ] **5-2** GameService 구현 (세이브/로드)
- [ ] **5-3** RankingService 구현

### Phase 6: Timer + Ops
- [ ] **6-1** Timer 구현
- [ ] **6-2** ClientStub 테스트 시나리오

### Phase 7: 마무리
- [ ] **7** 정상 종료 + README

---

## 핵심 설계 결정 (면접 포인트)

### 패킷 헤더 (12바이트)
```cpp
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t size;       // 전체 패킷 크기 (헤더 포함)
    uint16_t id;         // PacketId enum
    uint64_t authToken;  // 인증 토큰 (0 = 미인증)
};
#pragma pack(pop)
```

### DB 비동기 큐
- IOCP 워커 스레드는 절대 DB 호출로 블로킹되면 안 됨
- 워커 → `DBTaskQueue::Push(lambda)` → DB 전용 스레드가 처리
- DB 완료 후 세션 응답 시 `weak_ptr<Session>` 만료 확인 필수

### 랭킹 캐시
- `ranking_cache` 테이블: 60초마다 타이머로 갱신하는 materialized view
- 읽기 빈도 >> 쓰기 빈도이므로 O(1) 읽기를 위해 캐시 분리

### 세션 레퍼런스 카운팅
- 동시에 여러 overlapped I/O가 pending 가능
- 모두 완료되기 전까지 Session이 해제되면 크래시 → ref count 필수

---

## DB 스키마
```sql
CREATE TABLE accounts (
    account_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    username      TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,  -- SHA256(password + salt)
    salt          TEXT NOT NULL,
    created_at    INTEGER NOT NULL,
    last_login    INTEGER
);
CREATE TABLE game_saves (
    account_id    INTEGER PRIMARY KEY,
    save_data     TEXT NOT NULL,  -- JSON blob
    floor_reached INTEGER NOT NULL DEFAULT 0,
    updated_at    INTEGER NOT NULL,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);
CREATE TABLE runs (
    run_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id    INTEGER NOT NULL,
    character_class TEXT NOT NULL,
    floor_reached INTEGER NOT NULL,
    score         INTEGER NOT NULL,
    play_time_sec INTEGER NOT NULL,
    completed_at  INTEGER NOT NULL
);
CREATE TABLE ranking_cache (
    rank          INTEGER NOT NULL,
    account_id    INTEGER NOT NULL,
    username      TEXT NOT NULL,
    score         INTEGER NOT NULL,
    floor_reached INTEGER NOT NULL,
    cached_at     INTEGER NOT NULL
);
CREATE INDEX idx_runs_score ON runs(score DESC);
```

---

## 다음 작업
**Phase 1-2: SpinLock 구현** → `Shared/Core/SpinLock.h`
