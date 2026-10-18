# Stack Pack: Backend — Go (Gin/Echo/stdlib)

> Nạp file này khi `harness.config.yaml → surfaces.backend.stack: go-gin | go-echo | go-stdlib`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
API backend bằng Go.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `go test ./...` |
| Vet | `go vet ./...` |
| Lint | `golangci-lint run` |
| Build | `go build ./...` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Mọi error trả về đều được xử lý (`if err != nil`) — không `_ = err` trừ khi có comment giải thích.
- Context (`context.Context`) truyền xuyên suốt request path để hỗ trợ cancellation/timeout.
- Không dùng global mutable state không có mutex khi truy cập concurrent.
- SQL: dùng parameterized query (`database/sql` placeholders) hoặc query builder — không nối chuỗi.

## Lỗi thường gặp agent hay mắc (tránh)
- Goroutine leak do thiếu cancellation khi caller đã bỏ cuộc.
- Race condition không phát hiện được vì thiếu chạy `go test -race` trong CI.
- Shadowing biến `err` trong block `if` làm mất lỗi thật.

## Quality gate bổ sung
Xem `quality-gates/backend-gates.md`. Bổ sung: CI phải chạy `go test -race ./...`.

## Tham chiếu
- https://go.dev/doc/effective_go
