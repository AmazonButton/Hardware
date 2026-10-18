---
name: frontend-ui-design
description: Portable UI design system and engineering guidelines for modern web applications. Emphasizes typography, curated design tokens, dynamic animations, accessibility (WCAG 2.1 AA), and avoiding generic templates. Activates when designing or implementing web pages, dashboards, or components.
---

# Frontend UI Design System Skill

## Core Philosophy
1. **Never Generic**: Reject unstyled defaults, raw unadjusted Tailwind palettes (plain blue-500/red-500), and cookie-cutter templates.
2. **Intentional Hierarchy**: Establish clear visual contrast through scale, font weight, and muted surface elevations.
3. **Micro-Interactions**: Hover states, active pressed states, smooth accordion transitions, skeleton loaders for async queries.
4. **Accessible by Design**: All color pairings must meet WCAG 2.1 AA contrast ratios (4.5:1 for normal text, 3:1 for large text).

---

## Design Tokens Discipline

### Color Architecture (HSL Tokenization)
Always define semantic color tokens via CSS custom properties:
```css
:root {
  --background: 210 40% 98%;
  --foreground: 222.2 84% 4.9%;
  --card: 0 0% 100%;
  --card-foreground: 222.2 84% 4.9%;
  --popover: 0 0% 100%;
  --popover-foreground: 222.2 84% 4.9%;
  --primary: 221.2 83.2% 53.3%;
  --primary-foreground: 210 40% 98%;
  --secondary: 210 40% 96.1%;
  --secondary-foreground: 222.2 47.4% 11.2%;
  --muted: 210 40% 96.1%;
  --muted-foreground: 215.4 16.3% 46.9%;
  --accent: 210 40% 96.1%;
  --accent-foreground: 222.2 47.4% 11.2%;
  --destructive: 0 84.2% 60.2%;
  --destructive-foreground: 210 40% 98%;
  --border: 214.3 31.8% 91.4%;
  --input: 214.3 31.8% 91.4%;
  --ring: 221.2 83.2% 53.3%;
  --radius: 0.5rem;
}
```

### Typography Hierarchy
- **Primary Display Font**: Modern geometric sans (e.g., Inter, Plus Jakarta Sans, Outfit).
- **Monospace Font**: JetBrains Mono, Fira Code (for code snippets, hashes, IDs).
- **Scale**:
  - `h1`: 32px / line-height 1.2 / font-bold / tracking-tight
  - `h2`: 24px / line-height 1.3 / font-semibold / tracking-tight
  - `h3`: 18px / line-height 1.4 / font-medium
  - `body`: 14px / line-height 1.5 / font-normal
  - `caption`: 12px / line-height 1.4 / font-medium / text-muted-foreground

---

## Component Layout Rules
1. **Containers**: Max-width with consistent gutters (`max-w-7xl mx-auto px-4 sm:px-6 lg:px-8`).
2. **Card Elevation**: Border-first approach (`border border-border/60 bg-card shadow-sm hover:shadow-md transition-shadow`).
3. **Empty States**: Never show a blank screen. Provide an illustration or icon, clear descriptive title, concise explanation, and primary action button.
4. **Async States**:
   - Always render `<Skeleton />` loaders mimicking the actual content layout during loading.
   - Distinct error alert with retry button when queries fail.
