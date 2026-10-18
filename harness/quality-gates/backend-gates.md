# Backend Quality Gates

Applies to: Backend API service (configured via `surfaces.backend` in `harness.config.yaml`)

---

## Gate 1: Unit Tests

**Tool:** Test Runner (JUnit, Go test, Pytest, Jest)  
**Threshold:** All tests pass, 0 failures  
**Blocks merge:** Yes  

```bash
cd [backend-dir]
[backend-test-command]
```

Test configuration requires:
- Tests run against isolated test configuration/DB
- No tests require production secrets (use mocks/stubs for 3rd-party services)

---

## Gate 2: Test Coverage

**Tool:** Coverage Reporter (JaCoCo, Go cover, Coverage.py, c8)  
**Threshold:** ≥ 60% line coverage on domain/service layer  
**Blocks merge:** Warning only (blocks at < 40%)  

```bash
cd [backend-dir]
[backend-coverage-command]
```

`pom.xml` JaCoCo configuration (minimum):
```xml
<plugin>
  <groupId>org.jacoco</groupId>
  <artifactId>jacoco-maven-plugin</artifactId>
  <configuration>
    <rules>
      <rule>
        <element>BUNDLE</element>
        <limits>
          <limit>
            <counter>LINE</counter>
            <value>COVEREDRATIO</value>
            <minimum>0.40</minimum>  <!-- 40% hard block, 60% warning -->
          </limit>
        </limits>
      </rule>
    </rules>
  </configuration>
</plugin>
```

---

## Gate 3: OWASP Dependency Check

**Tool:** OWASP Dependency Check Maven Plugin  
**Threshold:** 0 Critical CVEs, 0 High CVEs  
**Blocks merge:** Yes (critical), Warning (high)  

```bash
cd [backend-dir]
./mvnw org.owasp:dependency-check-maven:check -DfailBuildOnCVSS=9
# Report: target/dependency-check-report.html
```

Run on a schedule (weekly) even if no deps changed.

---

## Gate 4: Build Success

**Tool:** Build Tool (Maven, Gradle, Go build, tsc, etc.)  
**Threshold:** Clean build, no compilation errors  
**Blocks merge:** Yes  

```bash
cd [backend-dir]
[backend-build-command]
```

The resulting artifact/binary must be produceable without errors.

---

## Gate 5: Database Migration Integrity

**Tool:** Migration Tool (Flyway, Liquibase, Prisma, Alembic)  
**Threshold:** All migrations apply cleanly on clean schema  
**Blocks merge:** Yes  

Validation:
- New migration file has correct sequential version number / timestamp
- No existing migration files were modified (checksum verification)
- Migration runs successfully against test DB in CI

---

## Gate 6: Health Endpoint (Post-Deploy)

**Tool:** `curl` / HTTP Client  
**Threshold:** HTTP 200 with status UP / healthy  
**Blocks merge:** N/A (post-deploy check)  

```bash
curl --fail [health-check-url]
# Expected: 200 OK {"status":"UP"}
```

---

## Gate 7: N+1 Detection & Query Auditing

**Tool:** Manual review + query logging / ORM statistics  
**Threshold:** 0 new N+1 patterns or unindexed full table scans  
**Blocks merge:** Yes (if detected by Backend Engineer role review)  

---

## Commands Summary

```bash
# Quick: tests only
cd [backend-dir] && [backend-test-command]

# Full: tests + coverage + build
cd [backend-dir] && [backend-verify-command]

# Security: dependency check
cd [backend-dir] && [backend-audit-command]
```
