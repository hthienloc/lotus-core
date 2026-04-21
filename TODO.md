# TODO.md — es-engine: Project Standardization

> **Vai trò tài liệu này**: Checklist cho các agents tiếp theo. Mỗi task đủ nhỏ để hoàn thành trong 1 phiên. Đọc `ARCHITECTURE.md` và `ROADMAP.md` trước khi bắt đầu bất kỳ task nào.
>
> **Cách xác nhận hoàn thành**: Chạy `cd build && make lotus_tests && ./lotus_tests` — phải `All tests PASSED!`.

---

## 🔴 P0 — Blocking (phải xong trước khi release)

### 1. CMake: Thêm install target và build shared library

**File**: `CMakeLists.txt`  
**Mục tiêu**: Để `fcitx5-lotus` có thể `find_package(lotus_engine)` và link chuẩn.

- [ ] Thêm `add_library(lotus_engine SHARED ...)` song song với `STATIC`
- [ ] Thêm `install(TARGETS lotus_engine_core DESTINATION lib)`
- [ ] Thêm `install(DIRECTORY include/ DESTINATION include)`
- [ ] Tạo `lotus_engine-config.cmake` để hỗ trợ CMake `find_package`
- [ ] Kiểm tra: `cmake --install build --prefix /tmp/lotus_install && ls /tmp/lotus_install/lib`

---

### 2. C-API: Hoàn thiện và test `libvnengine`

**File**: `src/capi.cpp`, `include/lotus_engine/capi.h`  
**Mục tiêu**: API ổn định cho tích hợp `fcitx5-lotus` (C linkage, opaque handles).

- [ ] Review `capi.h` — đảm bảo tất cả functions có `/** @brief ... */` Doxygen
- [ ] Thêm `lotus_engine_set_tone_style(engine, style)` vào C-API nếu chưa có
- [ ] Thêm `lotus_engine_reset(engine)` để reset state giữa các từ
- [ ] Viết `tests/test_capi_integration.cpp` — test full round-trip qua C interface
- [ ] Build `.so` và kiểm tra symbols: `nm -D build/liblotus_engine.so | grep lotus_engine`

---

### 3. Cập nhật ROADMAP.md thành trạng thái thực tế

**File**: `ROADMAP.md`  
**Mục tiêu**: ROADMAP đang ghi Phase 3 là "Active" nhưng thực ra đã xong.

- [ ] Đánh dấu `[x]` cho: Unicode NFC Normalization, Tone Placement Styles, C-API Wrapper
- [ ] Di chuyển "Active Phase" lên Fcitx5 Integration
- [ ] Cập nhật "Future Optimization" với benchmark targets cụ thể (< 1ms per keypress)

---

## 🟡 P1 — Important (nên xong trước khi merge lên main)

### 4. Logging Infrastructure

**File**: `include/lotus_engine/log.h` (tạo mới)  
**Mục tiêu**: Thay `printf/stdout` bằng structured logging để frontend capture được.

- [x] Tạo `log.h` với macro `VN_LOG_DEBUG`, `VN_LOG_INFO`, `VN_LOG_ERROR`
- [x] Default: log goes to `stderr`. Frontend có thể override qua callback.
- [x] Thêm `lotus_engine_set_log_callback(fn)` vào C-API
- [x] Xoá tất cả `printf(...)` debug trong production code (giữ trong tests ok)
- [x] Kiểm tra: `VNDEBUG=1 ./build/lotus_tui` hiển thị debug, mặc định thì không

---

### 5. CI Pipeline cơ bản

**File**: `.github/workflows/ci.yml` (tạo mới)  
**Mục tiêu**: Đảm bảo mọi PR đều chạy tests tự động.

- [x] Tạo workflow: `ubuntu-latest`, `cmake -B build`, `make lotus_tests`, `./lotus_tests`
- [x] Thêm build với `-Wall -Wextra -Werror` để bắt warnings
- [x] Optional: Matrix build với `clang` và `gcc`
- [x] Badge `![CI](https://github.com/.../actions/badge.svg)` vào README

---

### 6. README.md — Tạo tài liệu dự án

**File**: `README.md` (hiện chưa có)  
**Mục tiêu**: Onboarding cho contributors mới và fcitx5-lotus team.

- [x] Mô tả: es-engine là gì, dùng cho ai
- [x] Quick Start: clone → cmake → make → ./lotus_tests
- [x] Section: "Tích hợp vào dự án của bạn" (dùng C-API)
- [x] Section: "Kiến trúc" (link tới ARCHITECTURE.md)
- [x] Badge: CI status, version

---

## 🟢 P2 — Nice to Have (future optimization)

### 7. Performance Benchmark

**File**: `tests/bench_engine.cpp` (tạo mới)

- [ ] Benchmark `engine.process_key()` cho 10,000 keystrokes liên tiếp
- [ ] Target: < 1ms per keypress trên hardware phổ thông
- [ ] Output CSV để theo dõi regression

---

### 8. Memory Tuning — Static Lookup Tables

**File**: `src/validator.cpp`, `include/lotus_engine/unicode.h`

- [ ] Chuyển các `std::unordered_set<std::string>` sang `static constexpr`
- [ ] Profile với `valgrind --tool=massif` để đo heap usage
- [ ] Target: < 512KB total RAM footprint

---

### 9. Diphthong Coverage mở rộng

**File**: `src/syllable.cpp`, `tests/test_phonology.cpp`

- [ ] Kiểm tra `uơ` (khác với `ươ`) cho các từ như `mượn`, `lướt`
- [ ] Thêm test cho triphthong `iêu`, `uôi`, `ươi`, `ươu`
- [ ] Đảm bảo tone placement đúng cho tất cả cases

---

## 📋 Tham khảo nhanh

| File | Vai trò |
|------|---------|
| `include/lotus_engine/unicode.h` | NFC normalization, case, strip_tone |
| `include/lotus_engine/types.h` | Syllable, EngineResult, Tone, ToneStyle |
| `include/lotus_engine/capi.h` | C-API extern "C" interface |
| `src/engine.cpp` | 7-stage pipeline orchestrator |
| `src/syllable.cpp` | Tone placement heuristics |
| `src/validator.cpp` | Phonotactic rule enforcement |
| `src/parser.cpp` | Syllable structure parsing |
| `tests/tui_demo.cpp` | Manual testing TUI |

**Build commands:**

```bash
cmake -B build && cd build && make lotus_tests && ./lotus_tests  # Full test
make lotus_tui && ./lotus_tui                                   # Manual TUI
make lotus_tui && ./lotus_tui --vni                              # VNI mode
```
