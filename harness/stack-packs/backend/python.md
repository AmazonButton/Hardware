# Stack Pack: Backend — Python (FastAPI/Django)

> Nạp file này khi `harness.config.yaml → surfaces.backend.stack: python-fastapi | python-django`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
API backend bằng Python.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `pytest` |
| Lint | `ruff check .` |
| Type check | `mypy .` |
| Migration | `alembic upgrade head  # hoặc: python manage.py migrate` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Type hints bắt buộc cho function signature mới — `mypy` phải pass.
- FastAPI: input/output qua Pydantic models — không nhận `dict` thô ở boundary công khai.
- DB access qua ORM (SQLAlchemy/Django ORM) với parameterized query — không f-string vào SQL.
- Blocking I/O (requests đồng bộ, file lớn) không chạy trong async def route — dùng `run_in_threadpool` hoặc thư viện async.

## Lỗi thường gặp agent hay mắc (tránh)
- Mutable default argument (`def f(x=[])`) — lỗi kinh điển Python.
- N+1 query Django ORM khi thiếu `select_related`/`prefetch_related`.
- Không set timeout cho HTTP client ra ngoài → treo request vô hạn.

## Quality gate bổ sung
Xem `quality-gates/backend-gates.md`.

## Tham chiếu
- https://fastapi.tiangolo.com
- https://docs.djangoproject.com
