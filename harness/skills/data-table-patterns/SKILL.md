---
name: data-table-patterns
description: Standard architecture for production-grade data tables using TanStack Table v8, shadcn/ui Table, server-side/client-side pagination, sorting, search debounce, and row actions. Activates when implementing or refactoring tables in admin web dashboards.
---

# Production Data Table Skill

## Core Architecture
High-volume data tables must be resilient, accessible, and performant:
1. **Server-Side vs Client-Side**:
   - Small datasets (< 200 items): Client-side sorting, filtering, and pagination.
   - Large datasets (> 200 items): Server-driven pagination with URL query synchronization.
2. **URL State Synchronization**: Keep page, pageSize, search filter, and sort key in the URL (`useSearchParams`) so views are shareable and survive page refreshes.
3. **Debounced Search**: Input searches must be debounced by 300-400ms to prevent request thrashing.

---

## Standard Table Structure

```tsx
interface DataTableProps<TData, TValue> {
  columns: ColumnDef<TData, TValue>[]
  data: TData[]
  pageCount?: number
  pageSize?: number
  pageIndex?: number
  onPaginationChange?: (page: number, size: number) => void
  isLoading?: boolean
  searchKey?: string
}
```

## Mandatory UI States

### 1. Loading State
Render skeleton rows matching the exact column count:
```tsx
{isLoading && (
  Array.from({ length: 5 }).map((_, i) => (
    <TableRow key={i}>
      {columns.map((col, j) => (
        <TableCell key={j}><Skeleton className="h-6 w-full" /></TableCell>
      ))}
    </TableRow>
  ))
)}
```

### 2. Empty State
Render a centered empty state when query returns `[]`:
```tsx
{!isLoading && data.length === 0 && (
  <TableRow>
    <TableCell colSpan={columns.length} className="h-48 text-center">
      <div className="flex flex-col items-center justify-center text-muted-foreground">
        <FolderArchive className="h-10 w-10 stroke-1 mb-2" />
        <p className="font-medium">No results found</p>
        <p className="text-sm">Try adjusting your search or filter parameters.</p>
      </div>
    </TableCell>
  </TableRow>
)}
```

### 3. Column Actions & Menu
Always right-align action columns (View, Edit, Delete) with a dropdown menu (`DropdownMenu` + `MoreHorizontal` icon).
Destructive actions (Delete, Ban) MUST prompt a confirmation modal before dispatching mutations.
