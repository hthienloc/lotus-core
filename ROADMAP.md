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

## Active Phase: Advanced Engine Capabilities

Mục tiêu: Đưa Lotus Engine lên tầm cao mới về độ thông minh và chính xác.

### 1. Surrounding Text & Context Awareness

- [ ] **State Reconstruction**: Phát triển logic tái cấu trúc state engine từ text thuần (`rebuild_from_text`).
- [ ] **Cursor-Aware Editing**: Hỗ trợ nạp syllable hiện tại vào engine khi di chuyển con trỏ, cho phép sửa lỗi tiếng Việt ở bất kỳ đâu trong từ.
- [ ] **Word-History integration**: Kết nối sâu hơn với buffer của trình soạn thảo để đồng bộ hóa trạng thái gõ.

### 2. Rigorous Phonotactic Validation

- [ ] **Matrix-based Validation**: Chuyển đổi từ regex sang ma trận CV/VC (Consonant-Vowel/Vowel-Consonant) toàn diện.
- [ ] **Spell-Prevention**: Ngăn chặn gõ các tổ hợp phím không có thực trong tiếng Việt ngay từ phím nhấn đầu tiên (ví dụ: `qqu`, `ww`).
- [ ] **Dialect Tuning**: Tùy chỉnh các quy tắc ghép vần theo vùng miền hoặc chuẩn từ điển.

### 3. Advanced Transformation Architecture

- [ ] **Structured Composition**: Di chuyển từ raw buffer sang đồ thị biến đổi (Transformation Graph) để quản lý undo/redo chính xác.
- [ ] **Dynamic Re-marking**: Tự động nhảy dấu (Tone Jumping) thông minh hơn khi thay đổi cấu trúc nucleus.
- [ ] **Memory Tuning**: Giảm RAM footprint (< 512KB) cho các thiết bị nhúng.

## Ongoing Maintenance

- [ ] **Integration Test Coverage**: Mở rộng bộ test tích hợp với các ứng dụng thực tế (Chromium, Firefox, LibreOffice).
- [ ] **Logging & CI**: Chuẩn hóa hệ thống log và hoàn thiện CI GitHub Actions.
- [ ] **Documentation**: Cập nhật tài liệu kỹ thuật cho các kỹ sư muốn tích hợp logic lõi.
