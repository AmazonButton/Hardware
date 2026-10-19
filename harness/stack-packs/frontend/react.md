# Stack Pack: Frontend — React (Vite/Next.js)

> Nạp file này khi `harness.config.yaml → surfaces.frontend_web.stack: react-vite-typescript | nextjs`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Web app/dashboard dùng React 18/19, TypeScript strict.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| TypeCheck | `npx tsc --noEmit` |
| Lint | `npx eslint . --max-warnings 0` |
| Unit test | `npm run test -- --run` |
| Build | `npm run build` |
| Dev | `npm run dev` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Data fetching qua TanStack Query / React Query hoặc Next.js Server Components — không fetch trực tiếp trong component body không kiểm soát.
- State toàn cục dùng Zustand/Redux Toolkit — không props-drilling quá 3 cấp.
- Không dùng `any` không có eslint-disable kèm lý do.
- Design tokens/Tailwind config — không inline style trừ khi động theo data.
- Mọi interactive element có `aria-label` hoặc label hiển thị (a11y gate).
- Next.js: phân biệt rõ Server Component vs Client Component (`'use client'` chỉ khi cần state/effect).

## Lỗi thường gặp agent hay mắc (tránh)
- Quên cleanup `useEffect` → memory leak / duplicate subscription.
- Re-render thừa do tạo object/array literal trong JSX props mỗi render.
- Bundle phình do import cả thư viện thay vì tree-shakable import.
- Hydration mismatch khi dùng `Date.now()`/`Math.random()` trực tiếp trong render (Next.js SSR).

## Quality gate bổ sung
Xem `quality-gates/frontend-gates.md`. Bổ sung: Lighthouse Performance ≥ 90, bundle size diff báo cáo trong PR.

## Tham chiếu
- https://react.dev
- https://nextjs.org/docs
- roles/frontend-engineer.md
