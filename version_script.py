import subprocess

Import("env")

try:
    version = subprocess.check_output(
        ["git", "describe", "--tags", "--always"],
        stderr=subprocess.DEVNULL
    ).strip().decode("utf-8")
except Exception:
    version = "dev"

env.Append(CPPDEFINES=[
    ("FIRMWARE_VERSION", env.StringifyMacro(version))
])
