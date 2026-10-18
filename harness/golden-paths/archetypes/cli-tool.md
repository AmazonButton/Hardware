# Golden Path: CLI Tool Archetype

## 1. Overview
Dành cho công cụ dòng lệnh (developer tool, ops utility, code generator, migration tool) — phân phối dưới dạng binary/package, không có backend server thường trực.

---

## 2. Recommended Tech Stack
- **Option A (Systems/Performance)**: Rust (clap for arg parsing) hoặc Go (cobra)
- **Option B (Scripting ecosystem)**: Node.js/TypeScript (commander/oclif) hoặc Python (typer/click)
- **Packaging**: cargo/crates.io, npm, PyPI, hoặc binary release qua GitHub Releases
- **Testing**: snapshot testing cho output CLI (insta cho Rust, jest snapshot cho Node)

---

## 3. Directory Layout (tham khảo, ngôn ngữ trung lập)

```
├── src/
│   ├── commands/          # 1 file / 1 subcommand
│   ├── config/            # đọc config file, biến môi trường
│   ├── core/              # logic nghiệp vụ, tách khỏi lớp CLI I/O
│   └── output/            # format output (text/json/table)
├── tests/
│   ├── unit/
│   └── snapshot/          # golden-file output tests
├── docs/
│   └── usage.md           # `--help` output phải khớp file này
└── AGENTS.md
```

---

## 4. Quality Gates Checklist

| Stage | Command | Threshold |
| :--- | :--- | :--- |
| Unit tests | `cargo test` / `npm test` | 100% pass |
| Snapshot output | `cargo insta test` / `jest --ci` | Không lệch golden file (hoặc đã review + accept) |
| Cross-platform build | build cho Linux/macOS/Windows | Build thành công cả 3 |
| Binary size | kiểm tra kích thước binary | Không tăng > 20% giữa 2 release liên tiếp mà không có lý do |
| Checksum & Supply-chain | tạo SHA-256 cho mọi binary release | **Bắt buộc** — xem `scripts/bin/SECURITY_NOTE.md` |
| Help text đồng bộ | diff `--help` output với `docs/usage.md` | Khớp 100% |

## 5. Lưu ý đặc thù CLI
- Không bao giờ phân phối binary đã build sẵn mà không kèm mã nguồn tương ứng + checksum công khai (bài học từ chính `harness-cli.exe` trong bộ này — xem `GAP_ANALYSIS_AND_ROADMAP.md` mục 0).
- Exit code phải nhất quán: `0` thành công, khác `0` cho từng nhóm lỗi cụ thể (không dùng chung 1 mã lỗi cho mọi trường hợp).
