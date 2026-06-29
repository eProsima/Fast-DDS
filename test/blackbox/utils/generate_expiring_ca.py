# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pathlib import Path
import os
import shutil
import subprocess
import sys
from datetime import datetime, timedelta, timezone


def find_certs_dir():
    # Preferred: CTest sets CERTS_PATH when invoking tests, pointing at the real source certs dir.
    env_path = os.environ.get("CERTS_PATH")
    if env_path:
        candidate = Path(env_path)
        if candidate.exists():
            return candidate

    # Fallback for manual invocation: walk up from __file__ looking for a certs dir
    # that contains the expected modifiablesubreq.pem.
    for ancestor in Path(__file__).resolve().parents:
        candidate = ancestor / "certs"
        if (candidate / "modifiablesubreq.pem").exists():
            return candidate

    return None


def _build_openssl_env():
    env = os.environ.copy()

    if sys.platform == "win32":
        program_files = env.get("ProgramW6432") or env.get("ProgramFiles", r"C:\Program Files")
        extra_dirs = [
            os.path.join(program_files, "OpenSSL", "bin"),
            os.path.join(program_files, "OpenSSL-Win", "bin"),
            os.path.join(program_files, "OpenSSL-Win64", "bin"),
            os.path.join(program_files, "OpenSSL"),
            os.path.join(program_files, "OpenSSL-Win"),
            os.path.join(program_files, "OpenSSL-Win64"),
        ]
        search_path = env.get("PATH", "") + os.pathsep + os.pathsep.join(extra_dirs)
        exe = shutil.which("openssl", path=search_path)
        if exe:
            exe_dir = os.path.dirname(exe)
            if exe_dir not in env.get("PATH", ""):
                env["PATH"] = env.get("PATH", "") + os.pathsep + exe_dir
            return exe, env
        return None, env

    return shutil.which("openssl"), env


def run_openssl(args, cwd=None):
    openssl_exe, env = _build_openssl_env()
    if openssl_exe is None:
        print(
            "[-] openssl executable not found.\n"
            "    Ensure openssl is installed and on PATH.",
            file=sys.stderr,
        )
        sys.exit(1)

    cmd = [openssl_exe] + args
    try:
        subprocess.run(cmd, check=True, cwd=cwd, env=env,
                       stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(f"[-] openssl command failed: {' '.join(str(a) for a in cmd)}\n"
              f"    Error: {e.stderr.decode().strip()}")
        raise


if __name__ == "__main__":
    certs_dir = find_certs_dir()
    if not certs_dir:
        print(f"[-] Certificates directory not found (searched relative to {Path(__file__).resolve()})")
        sys.exit(1)

    print(f"[*] Targeting directory: {certs_dir}")

    # --- 1. Remove any existing Modifiable Subscriber entry from index.txt ---
    index_file = certs_dir / "index.txt"
    if index_file.exists():
        lines = index_file.read_text().splitlines(keepends=True)
        filtered = [l for l in lines if "CN=Modifiable Subscriber" not in l]
        index_file.write_text("".join(filtered))
        print("[*] Cleaned Modifiable Subscriber entry from index.txt")

    # --- 2. Compute expiration: now + 20 seconds ---
    expiry = datetime.now(timezone.utc) + timedelta(seconds=20)
    # openssl ca -enddate expects YYMMDDHHMMSSZ (UTCTime)
    enddate_str = expiry.strftime("%y%m%d%H%M%S") + "Z"
    print(f"[*] Certificate will expire at {enddate_str} (UTC)")

    # --- 3. Sign modifiablesubreq.pem with a 20-second validity ---
    print("[*] Signing modifiablesubcert.pem with 20-second expiry...")
    run_openssl(
        [
            "ca", "-batch", "-create_serial",
            "-config", "maincaconf.cnf",
            "-enddate", enddate_str,
            "-in", "modifiablesubreq.pem",
            "-out", "modifiablesubcert.pem",
        ],
        cwd=certs_dir,
    )

    print("[+] Done. modifiablesubcert.pem will expire in ~20 seconds.")