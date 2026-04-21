# Project Roadmap: Lotus Engine

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
  - Export C-API (`liblotus_engine_core.so`) ổn định cho `fcitx5-lotus`.

## Active Phase: State Reconstruction & Context Awareness

Mục tiêu: Đưa Lotus Engine lên tầm cao mới về độ thông minh và trải nghiệm người dùng thực tế.

### 1. Surrounding Text & State Reconstruction

- [x] **State Reconstruction**: Phát triển logic tái cấu trúc trạng thái engine từ text thuần (`rebuild_from_text`). Cho phép nạp phím gõ từ một từ đã có sẵn.
- [x] **Cursor-Aware Editing**: Hỗ trợ nạp syllable hiện tại vào engine khi di chuyển con trỏ, cho phép sửa dấu/typo ở bất kỳ đâu trong từ.
- [ ] **Fcitx5 Surrounding Text**: Tích hợp sâu với API surrounding-text của Fcitx5 để đồng bộ hóa trạng thái gõ và xóa.

### 2. Rigorous Validation & Maintenance

- [ ] **Enhanced Validation**: Cải thiện `Validator` để ngăn chặn gõ các tổ hợp phím vô lý (ví dụ: `qqu`, `ww`) mà không cần làm quá phức tạp kiến trúc.
- [ ] **Integration Test Coverage**: Mở rộng bộ test tích hợp với các ứng dụng thực tế (Chromium, Firefox, LibreOffice) để giải quyết các lỗi mất phím.
- [ ] **CI & Automation**: Hoàn thiện luồng CI GitHub Actions và chuẩn hóa hệ thống logging.

## Long-term Research (Pha tương lai)

- [ ] **Matrix-based Validation**: Chuyển đổi sang ma trận CV/VC nếu cần độ chính xác 100% về phương ngữ.
- [ ] **Memory & Performance**: Tiếp tục tối ưu RAM footprint (< 512KB) và latency.
- [ ] **Structured Composition Graph**: Chỉ thực hiện nếu phát hiện các giới hạn của buffer phẳng trong việc xử lý Undo/Redo phức tạp.
