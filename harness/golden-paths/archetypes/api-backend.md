# Golden Path: API Backend Service Archetype

## 1. Overview
The **API Backend Service Archetype** is built for high-throughput, mission-critical backend APIs using Spring Boot (Java), Go (Gin/Fiber), or Python (FastAPI), backed by relational databases and caching layers.

---

## 2. Recommended Tech Stack
- **Framework Options**:
  - **Option A (Enterprise Java)**: Spring Boot 3.4+ / Java 21 LTS (Virtual Threads enabled)
  - **Option B (Cloud-Native Go)**: Go 1.23+ with Gin / Fiber
  - **Option C (AI / Data Python)**: FastAPI / Python 3.12 with Pydantic v2
- **Database**: PostgreSQL 16+ with Flyway or Liquibase migrations
- **Caching & Broker**: Redis 7+ / Apache Kafka or RabbitMQ
- **Testing**: JUnit 5 + Testcontainers (Java) / Go test + dockertest / Pytest + testcontainers
- **Observability**: Prometheus + OpenTelemetry + Grafana Actuator

---

## 3. Directory Layout (Spring Boot Reference)

```
├── src/
│   ├── main/
│   │   ├── java/com/example/api/
│   │   │   ├── config/           # Security, CORS, Redis, OpenTelemetry
│   │   │   ├── controller/       # REST API endpoints (thin controllers)
│   │   │   ├── dto/              # Request & Response records/contracts
│   │   │   ├── entity/           # JPA entities / Database tables
│   │   │   ├── exception/        # GlobalExceptionHandler & domain errors
│   │   │   ├── repository/       # Spring Data JPA / QueryDSL interfaces
│   │   │   └── service/          # Core domain business logic
│   │   └── resources/
│   │       ├── db/migration/     # Flyway SQL migration scripts (V1__, V2__)
│   │       └── application.yml   # Spring environment configuration
│   └── test/
│       ├── java/com/example/api/ # Unit & Testcontainers integration tests
└── AGENTS.md                     # Surface-specific agent instructions
```

---

## 4. Quality Gates Checklist

| Stage | Command | Threshold |
| :--- | :--- | :--- |
| **Unit & Slice Tests** | `./mvnw test` | 100% pass |
| **Integration Tests** | `./mvnw verify -Pintegration` | Real DB container verification |
| **Test Coverage** | `JaCoCo report` | ≥ 80% line coverage on services |
| **Dependency Audit** | `./mvnw dependency-check:check` | 0 High/Critical CVEs |
| **Container Build** | `docker build -t api:latest .` | Multi-stage distroless build |
| **Health Probe** | `curl -f /actuator/health` | Status UP |
