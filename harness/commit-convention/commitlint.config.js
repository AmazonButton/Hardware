// commitlint.config.js
// Conventional Commits configuration for Harness Engineering Framework
// See: https://commitlint.js.org/

module.exports = {
  extends: ['@commitlint/config-conventional'],

  rules: {
    // Type must be one of the allowed types
    'type-enum': [
      2, // level: error
      'always',
      [
        'feat',      // New feature
        'fix',       // Bug fix
        'docs',      // Documentation
        'style',     // Formatting (no behavior change)
        'refactor',  // Code restructure
        'perf',      // Performance improvement
        'test',      // Tests only
        'build',     // Build system / dependencies
        'ci',        // CI/CD configuration
        'chore',     // Maintenance
        'harness',   // Harness engineering changes
        'revert',    // Revert previous commit
      ],
    ],

    // Type must be lowercase
    'type-case': [2, 'always', 'lower-case'],

    // Scope must be lowercase
    'scope-case': [2, 'always', 'lower-case'],

    // Scope must be one of the allowed scopes (warning only — scopes can evolve)
    'scope-enum': [
      1, // level: warning
      'always',
      [
        'backend',
        'mobile',
        'admin',
        'infra',
        'harness',
        'docs',
        'deps',
        'security',
        'legal',
      ],
    ],

    // Subject (short description) must be lowercase
    'subject-case': [2, 'always', 'lower-case'],

    // Subject must not end with period
    'subject-full-stop': [2, 'never', '.'],

    // Subject must not be empty
    'subject-empty': [2, 'never'],

    // Maximum total header length: 100 chars
    'header-max-length': [2, 'always', 100],

    // Body lines max 100 chars (for readability)
    'body-max-line-length': [1, 'always', 100],

    // Footer lines max 100 chars
    'footer-max-line-length': [1, 'always', 100],
  },

  // Ignore merge commits and revert commits (auto-generated)
  ignores: [
    (commit) => commit.startsWith('Merge '),
    (commit) => commit.startsWith('Revert '),
    (commit) => commit.startsWith("Merge branch '"),
  ],
};
