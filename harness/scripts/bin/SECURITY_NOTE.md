# ✅ Resolved in v3 — `harness-cli.exe` binary removed

**v0.2.0 và trước đó** phân phối `harness-cli`/`harness-cli.exe` như một binary Rust đã
biên dịch sẵn (~2.7MB), **không kèm mã nguồn** trong bundle. Đây từng là gap bảo mật ưu
tiên #1 (xem `V3_UPGRADE_REPORT.md § 1`): mọi agent/CI thực thi một binary không thể audit.

**Kể từ v3.0.0, binary này đã bị GỠ BỎ khỏi bộ harness.** CLI mặc định giờ là
[`scripts/harness_cli.py`](../harness_cli.py) — pure Python 3.9+ stdlib (`sqlite3`,
`argparse`), zero dependency, ~350 dòng, đọc được toàn bộ trong 1 lần review code. Không có
gì để build, không có checksum cần verify — mã nguồn CHÍNH LÀ artifact chạy.

```bash
python3 scripts/harness_cli.py init
python3 scripts/harness_cli.py query matrix
```

## Nếu bạn cần hiệu năng cao hơn (dự án rất lớn, gọi CLI hàng nghìn lần/ngày)

`harness_cli.py` đủ nhanh cho phần lớn dự án (SQLite local, không network). Nếu thực sự
cần một bản Rust/Go compiled nhanh hơn:

1. Bắt buộc đưa **mã nguồn đầy đủ** vào repo (vd `scripts/harness-cli-src/`), không chỉ
   binary.
2. Build bằng CI, publish checksum SHA-256 kèm mỗi release trong `CHANGELOG.md`.
3. `scripts/checks/check_cli_checksum.sh` vẫn còn trong repo — cập nhật giá trị checksum
   mong đợi trong đó nếu bạn tự build và muốn CI verify trước khi thực thi.
4. Tool phải implement đúng interface của `harness_cli.py` (xem docstring trong file đó)
   để không phá vỡ các lệnh mà roles/*.md và templates/*.md đang tham chiếu.

Cho tới khi điều kiện trên được đáp ứng, **không** đưa lại một binary không có mã nguồn
vào `scripts/bin/` — đây chính là gap đã được vá trong v3, đừng tái tạo nó.
