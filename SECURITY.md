# Security Policy

## Supported Versions

Only the latest release receives security fixes.

## Reporting a Vulnerability

Please **do not** open a public GitHub issue for security vulnerabilities.

Use [GitHub's private vulnerability reporting](https://github.com/partoska/p6a-cmd/security/advisories/new) to submit a report. You can expect:

- Acknowledgement within **168 hours**.
- A fix or mitigation plan within **90 days**, depending on severity.
- Credit in the release notes if you would like it.

## Scope

This tool runs entirely on your local machine and communicates only with the Partoska API using OAuth 2.0 tokens stored in `~/.p6a/` with restricted file permissions (`0600`). If you find an issue related to token handling, credential exposure, or unsafe API communication, please report it.
