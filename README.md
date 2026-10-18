# C/C++와 Rust 메모리 안전성 실행 예제

6개 유형을 **안전하게 실행되는 수정 코드**, **Rust 컴파일 실패 코드**, **C++의 의도적인 오류 코드**로 비교합니다. C++17과 Rust 2021 edition을 사용하며 외부 라이브러리나 Cargo 의존성이 없습니다. C++ 오류 예제의 `malloc/free`, `strcpy` 부분은 C에서도 같은 위험이 있는 API입니다.

## 준비 및 실행

Linux 또는 WSL에서 Bash, `g++`(C++17·sanitizer 지원), `rustc`를 준비하세요. Rust는 https://rustup.rs 의 설치 안내를 따를 수 있습니다.

```bash
bash scripts/run.sh safe        # C++/Rust 수정 코드 실행 + assert 검증
bash scripts/run.sh check-fail  # 8개 예제의 컴파일 실패와 예상 진단 확인
bash scripts/run.sh panic       # 동적 인덱스의 범위 초과 panic 확인
```

`safe`는 성공하면 `C++ safe examples passed`, `Rust safe examples passed`를 출력합니다. `check-fail`은 컴파일 실패가 정상이며 각 항목에 `PASS`를 출력합니다. `panic`은 자식 프로세스의 실패를 확인한 뒤 스크립트 자체는 성공합니다. 출력·진단 문구는 컴파일러 버전에 따라 조금씩 달라질 수 있습니다. 로그와 실행 파일은 `build/`에 생성됩니다.

C++ 취약 코드는 아래처럼 **각각 별도 프로세스**로 실행합니다. 실제 미정의 동작을 발생시키므로 연습 환경에서 실행하세요. sanitizer가 오류를 검출하면 0이 아닌 종료 코드가 나오는 것이 정상입니다.

```bash
bash scripts/run.sh unsafe uaf
bash scripts/run.sh unsafe double-free
bash scripts/run.sh unsafe iterator
bash scripts/run.sh unsafe overflow
bash scripts/run.sh unsafe race
bash scripts/run.sh unsafe leak
bash scripts/run.sh unsafe null
```

`race`는 ThreadSanitizer, 나머지는 AddressSanitizer와 UndefinedBehaviorSanitizer를 사용합니다. 일부 컨테이너·OS는 sanitizer 실행을 지원하지 않습니다. `unexpected memory mapping` 같은 초기화 실패는 버그 검출 결과가 아닙니다. 미정의 동작의 출력·크래시 여부는 보장되지 않으며, sanitizer도 모든 버그를 검출하지는 않습니다.

## 파일과 비교 항목

| 유형 | C++ 오류 (`cpp/unsafe.cpp` 실행 인자) | Rust 컴파일 실패 (`rust/compile_fail/`) | 수정 코드 |
|---|---|---|---|
| 해제 후 사용 | `uaf` | `uaf.rs`: E0382, `dangling.rs`: E0515 | 소유권을 유지하거나 소유한 `String` 반환 |
| 이중 해제 | `double-free` | `double_free.rs`: E0382 | RAII, 소유권 이동 |
| 반복자 무효화 | `iterator` | `iterator.rs`: E0502 | 추가할 값을 수집한 후 확장 |
| 버퍼 범위 초과 | `overflow` | `constant_bounds.rs`: 기본 deny lint | C++ `at`, Rust `get`, 별도 panic 예제 |
| 데이터 레이스 | `race` | `race.rs`: E0499, `rc_thread.rs`: E0277 | 원자적 카운터 + join |
| 누수 / 널 역참조 | `leak`, `null` | `option.rs`: E0308 | RAII, `optional`/`Option` |

전체 수정 코드는 `cpp/safe.cpp`, `rust/safe.rs`에 있습니다. `rust/bounds_panic.rs`는 입력 인덱스를 사용하므로 상수 인덱스 lint와 런타임 검사를 분리해서 보여줍니다. 단독 실행도 가능합니다.

```bash
rustc --edition=2021 rust/bounds_panic.rs -o build/bounds-panic
./build/bounds-panic 1   # 2 출력
./build/bounds-panic 10  # panic
```

## 원문 설명에서 주의할 점

- Rust의 메모리 안전성 보장은 **sound한 라이브러리·unsafe 구현을 전제로 한 safe Rust**에 적용됩니다. raw pointer, `unsafe`, FFI의 잘못된 구현으로 UAF·이중 해제 등을 만들 수 있습니다.
- 소유권 검사는 주로 컴파일 시 수행되지만 모든 안전성 검사가 컴파일 시 끝나지는 않습니다. 슬라이스 인덱싱에는 런타임 경계 검사가 있고, 제거 여부는 최적화가 증명할 수 있는지에 달려 있습니다. `Arc`의 원자 연산이나 `Mutex`의 동기화에도 비용이 있습니다.
- `let i = 10; arr[i]`도 컴파일러가 범위 초과를 증명하면 기본 `unconditional_panic` lint로 거부할 수 있습니다. 런타임 예제에는 명령행 입력을 사용했습니다. 이 lint는 억제할 수 있으며 경계 검사는 계속 작동합니다.
- panic은 프로세스를 항상 즉시 종료하지는 않습니다. unwind를 통해 잡을 수 있고, 빌드 설정에 따라 abort할 수 있습니다. 범위 밖 메모리 접근을 허용하지 않는 것이 핵심입니다.
- 소멸자는 정상적인 스코프 종료·early return·panic unwind 때 실행됩니다. abort, `process::exit`, 강제 종료에서는 보장되지 않습니다. `mem::forget`, `Box::leak`, `Rc` 순환 참조 등으로 safe Rust에서도 누수가 가능합니다. 누수는 메모리 안전성 위반으로 분류되지 않습니다.
- Rust에도 raw null pointer가 있습니다. 안전한 `&T`·`&mut T`는 null일 수 없으며 부재는 보통 `Option`으로 표현합니다. `Option`은 `unwrap()`으로 panic을 선택하거나 무시할 수도 있어 정상적인 분기 처리를 반드시 강제하는 것은 아닙니다. `Option<&T>`는 `&T`와 같은 크기이지만 모든 `Option<T>`가 그런 것은 아닙니다.
- C++의 `unique_ptr`, `optional`, `vector::at`, 원자 타입도 위험을 줄입니다. 다만 이들이 raw pointer·참조의 수명 오용 전체를 컴파일 시 막지는 않습니다.
- C++ 범위 for 루프에서 `push_back`은 재할당이 없어도 기존 past-the-end 반복자를 무효화합니다. 컨테이너별 규칙도 다릅니다. 예를 들어 `map::erase`는 삭제된 원소의 반복자·참조만 무효화합니다. 샘플은 `reserve`로 재할당을 강제해 오류를 재현합니다.
- 데이터 레이스는 단순한 증가분 유실을 넘어 C++의 미정의 동작입니다. 반대로 Rust가 데이터 레이스를 방지해도 교착 상태·논리적 경쟁 상태까지 제거하지는 않습니다. 샘플의 `Relaxed`는 카운트만 원자적으로 갱신하며 다른 데이터의 공개·순서를 보장하려는 용도가 아닙니다.
- Heartbleed는 버퍼 **과다 읽기** 사례입니다. 버퍼 쓰기 초과, 재귀 등에 의한 스택 고갈은 구분해야 합니다. 메모리 안전성 취약점의 “약 70%”는 특정 기관·제품·기간의 보고 수치이며 모든 취약점에 적용되는 보편적 비율이 아닙니다. Firefox 병렬화 “세 번 실패” 주장도 이 예제에서 검증한 사실이 아니므로 근거 없이 일반화하지 않습니다.

## 순수 C 예제

`cpp/unsafe_c.c`는 UAF·이중 해제·범위 초과·누수·널 역참조의 C11 버전입니다. 예를 들어:

```bash
mkdir -p build
cc -std=c11 -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer cpp/unsafe_c.c -o build/unsafe-c
./build/unsafe-c uaf  # 다른 인자: double-free, overflow, leak, null
```
