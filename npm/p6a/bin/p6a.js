#!/usr/bin/env node

/*
 * Command Line Interface for Partoska.com media sharing service.
 * Copyright (C) 2026 Fabrika Charvat s.r.o. All rights reserved.
 * Developed by Partoska Laboratory team, <https://lab.partoska.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * You can contact the author(s) via email at ask <at> partoska.com.
 */

"use strict";

const { spawnSync } = require("child_process");

// Map `${process.platform}-${process.arch}` to the package + relative binary.
const TARGETS = {
  "darwin-x64": ["@partoska/p6a-darwin", "bin/p6a"],
  "darwin-arm64": ["@partoska/p6a-darwin", "bin/p6a"],
  "linux-x64": ["@partoska/p6a-linux-x64", "bin/p6a"],
  "linux-arm64": ["@partoska/p6a-linux-arm64", "bin/p6a"],
  "win32-x64": ["@partoska/p6a-win32-x64", "bin/p6a.exe"],
  "win32-arm64": ["@partoska/p6a-win32-arm64", "bin/p6a.exe"],
};

const key = `${process.platform}-${process.arch}`;
const target = TARGETS[key];
if (!target) {
  console.error(
    `p6a: unsupported platform/architecture "${key}".\n` +
      `Supported: ${Object.keys(TARGETS).join(", ")}.\n`
  );
  process.exit(1);
}

const [pkg, relPath] = target;
let binary;
try {
  binary = require.resolve(`${pkg}/${relPath}`);
} catch {
  console.error(
    `p6a: could not find the native binary for "${key}".\n` +
      `The optional dependency "${pkg}" does not seem to be installed.\n` +
      `Try reinstalling p6a (e.g. "npm install -g @partoska/p6a"), and make\n` +
      `sure optional dependencies are not disabled.`,
  );
  process.exit(1);
}

const result = spawnSync(binary, process.argv.slice(2), { stdio: "inherit" });
if (result.error) {
  console.error(`p6a: failed to launch native binary: ${result.error.message}`);
  process.exit(1);
}

// Mirror the child's exit.
if (result.signal) {
  process.kill(process.pid, result.signal);
} else {
  process.exit(result.status === null ? 1 : result.status);
}
