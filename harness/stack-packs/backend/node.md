# Stack Pack: Backend — Node.js (NestJS/Express/Fastify)

> Nạp file này khi `harness.config.yaml → surfaces.backend.stack: node-nestjs | node-express | node-fastify`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
API backend bằng Node.js/TypeScript.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `npm test` |
| Lint | `npm run lint` |
| Build | `npm run build` |
| Migration | `npm run migrate` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Không dùng `any` — bật `strict` trong `tsconfig.json`.
- Validation input qua schema (Zod/class-validator) ở boundary — không tin request body trực tiếp.
- Query DB qua ORM/query builder có parameterization (Prisma/TypeORM/Kysely) — không nối chuỗi SQL.
- Mỗi migration là 1 file mới — không sửa migration đã chạy production.
- Async error handling: mọi route async phải có try/catch hoặc middleware bắt lỗi tập trung.

## Lỗi thường gặp agent hay mắc (tránh)
- Unhandled promise rejection làm crash process khi thiếu global handler.
- N+1 query khi dùng ORM relations không `include`/`join` đúng.
- Secrets đọc trực tiếp từ `process.env` rải rác thay vì 1 config module có validate.

## Quality gate bổ sung
Xem `quality-gates/backend-gates.md`.

## Tham chiếu
- https://docs.nestjs.com
