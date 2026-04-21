# Project Roadmap: es-engine

## Completed Milestones

- [x] **Giai đoạn 1: Vietnamese Phonology**
  - Robust Syllable Parser with Glide/Nucleus separation.
  - Component-based validation with strict orthic rules.
  - Exhaustive 173+ rhyme coverage.
- [x] **Giai đoạn 2: Transformation Pipeline**
  - Advanced 7-stage Telex transformer.
  - Full VNI method support.
  - Shortcut expansion system.
- [x] **Giai đoạn 3: State & Recovery**
  - Resilient backspace chaining across word boundaries.
  - Action-based feedback for immediate UI updates.
  - Foreign word auto-restore.
- [x] **Giai đoạn 4: Unicode Stability & Integration**
  - Đảm bảo output luôn ở chuẩn NFC để tránh lỗi hiển thị.
  - Xử lý các tổ hợp dấu kết hợp (Combining Marks).
  - Hỗ trợ đổi style bỏ dấu (Kiểu cũ `hòa` vs Kiểu mới `hoà`).
  - Export C-API (`libvnengine.so`) ổn định cho `fcitx5-lotus`.

## Active Phase: Project Standardization & Fcitx5 Integration

Mục tiêu: Đưa dự án đạt chuẩn sản xuất và gắn kết vào frontend.

- [ ] **Fcitx5 Integration**: Kết nối `libvnengine` (C-API) với frontend `fcitx5-lotus`.
- [ ] **Logging & CI**: Chuẩn hóa `log.h`, tạo luồng CI GitHub actions.
- [ ] **Tài liệu**: Cập nhật `README.md` hướng dẫn developer và users sử dụng thư viện.

## Future Optimization

- [ ] **Memory Tuning**: Giảm RAM footprint (< 512KB) bằng static trie hoặc perfect hashing.
- [ ] **Performance Benchmarking**: Đo latency pipeline, target < 1ms per keypress với 10k thao tác gõ.
