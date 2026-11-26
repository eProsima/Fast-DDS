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

import re
import sys
from datetime import datetime, timedelta, timezone

from generate_expiring_ca import find_certs_dir, run_openssl

PERMISSIONS_XML = "permissions_grant_expiration.xml"
PERMISSIONS_SMIME = "permissions_grant_expiration.smime"
EXPIRING_GRANT = "ExpiringSubscriberPermissions"


if __name__ == "__main__":
    certs_dir = find_certs_dir()
    if not certs_dir:
        print("[-] Certificates directory not found")
        sys.exit(1)

    print(f"[*] Targeting directory: {certs_dir}")

    # --- 1. Set the expiring grant's not_after to now + 20 seconds ---
    # Permissions dates without a time zone are UTC
    expiry = datetime.now(timezone.utc) + timedelta(seconds=20)
    not_after = expiry.strftime("%Y-%m-%dT%H:%M:%S")

    xml_file = certs_dir / PERMISSIONS_XML
    content = xml_file.read_text()
    pattern = re.compile(
        r'(<grant name="' + EXPIRING_GRANT + r'">.*?<not_after>)[^<]*(</not_after>)',
        re.DOTALL)
    content, count = pattern.subn(r'\g<1>' + not_after + r'\g<2>', content)
    if count != 1:
        print(f"[-] Grant {EXPIRING_GRANT} not found in {xml_file}")
        sys.exit(1)

    xml_file.write_text(content)
    print(f"[*] Grant {EXPIRING_GRANT} will expire at {not_after} (UTC)")

    # --- 2. Sign it with the permissions CA ---
    print(f"[*] Signing {PERMISSIONS_SMIME}...")
    run_openssl(
        [
            "smime", "-sign", "-text",
            "-in", PERMISSIONS_XML,
            "-out", PERMISSIONS_SMIME,
            "-signer", "maincacert.pem",
            "-inkey", "maincakey.pem",
        ],
        cwd=certs_dir,
    )

    print(f"[+] Done. {PERMISSIONS_SMIME} grant {EXPIRING_GRANT} will expire in ~20 seconds.")
