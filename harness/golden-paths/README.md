# Golden Paths Architecture

## 1. What is a Golden Path?
A **Golden Path** is an opinionated, pre-architected, and fully validated blueprint for a specific project archetype.

Rather than reinventing project structure, testing pipelines, and agent rules for each new repository, HEF provides ready-to-adopt Golden Paths across six archetypes:

1. **`web-fullstack`**: Next.js / React + Node.js/NestJS / Go + PostgreSQL.
2. **`mobile-only`**: Expo / React Native or Flutter cross-platform mobile client.
3. **`api-backend`**: High-performance backend API service (Spring Boot, Go/Gin, FastAPI).
4. **`mobile-fullstack`**: Multi-surface architecture combining Mobile Client, Web Admin, and Backend API in a unified monorepo.
5. **`cli-tool`**: `archetypes/cli-tool.md` — standalone command-line tool/utility.
6. **`data-pipeline`**: `archetypes/data-pipeline.md` — batch job / queue consumer / ETL worker.

---

## 1.5 Golden Path (kiến trúc/topology) vs Stack Pack (công nghệ cụ thể) — v3

Từ v3, một archetype không còn khoá cứng vào 1 công nghệ. Archetype = **topology** (có
bao nhiêu surface, chúng nói chuyện với nhau thế nào, CI pipeline hình dạng gì). Công nghệ
cụ thể cho từng surface (React hay Vue? Node hay Go?) đến từ `stack-packs/` (xem
`stack-packs/README.md`), chọn qua `harness.config.yaml → surfaces.<x>.stack`.

Ví dụ: `web-fullstack` với `frontend_web.stack: vue3-vite-typescript` +
`backend.stack: go-gin` là hợp lệ — archetype vẫn là `web-fullstack` (2 surface, 1 API
contract giữa chúng), chỉ khác stack-pack nào được nạp cho mỗi surface. Mỗi archetype file
trong `archetypes/*.md` nêu ví dụ stack minh hoạ, không phải danh sách đóng.

---

## 2. Structure of Each Golden Path
Every archetype definition contains:
- **Tech Stack & Toolchain specifications**: Exact recommended versions and tools.
- **Directory Layout**: Clean separation of domain logic, presentation, and data access.
- **Testing Pyramid & Quality Gates**: Required unit, integration, and E2E gates.
- **CI/CD Pipeline Mapping**: Ready-to-use GitHub Actions workflow.
- **Agent Instruction Rules (`AGENTS.md`)**: Customized prompts and constraints for that surface.
