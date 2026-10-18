# Stack Pack: Frontend — Svelte / SvelteKit

> Nạp file này khi `harness.config.yaml → surfaces.frontend_web.stack: svelte5-sveltekit`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Web app dùng Svelte 5 (runes) hoặc SvelteKit.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Check | `npm run check` |
| Lint | `npm run lint` |
| Test | `npm run test -- --run` |
| Build | `npm run build` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Svelte 5: dùng runes (`$state`, `$derived`, `$effect`) — tránh cú pháp store cũ cho state cục bộ mới.
- SvelteKit: load data qua `+page.ts`/`+page.server.ts`, không fetch trong component khi có thể load ở route.
- Form actions dùng `+page.server.ts` actions thay vì gọi API thủ công từ client khi có thể.

## Lỗi thường gặp agent hay mắc (tránh)
- Trộn lẫn runes và store cũ trong cùng component gây khó đọc/lỗi reactivity.
- Quên `export const prerender`/`ssr` khi route cần static hoặc client-only.

## Quality gate bổ sung
Xem `quality-gates/frontend-gates.md`.

## Tham chiếu
- https://svelte.dev/docs
