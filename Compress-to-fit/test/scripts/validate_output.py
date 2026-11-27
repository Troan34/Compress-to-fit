import subprocess, tempfile, filecmp, os, sys, random, shutil

compressor = sys.argv[1]

ITERATIONS = 100

for i in range(ITERATIONS):
    print(f"--- Iteration {i + 1}/{ITERATIONS} ---")

    seed = random.randrange(sys.maxsize)
    random.seed(seed)

    with tempfile.TemporaryDirectory() as temp_dir:
        original = os.path.join(temp_dir, "original.bin")
        with open(original, "wb") as dummy:
            dummy.write(random.randbytes(1024 * 1024)) # 1MB

        compressed = os.path.join(temp_dir, "compressed")
        restored = os.path.join(temp_dir, "restored")

        try:
            #compression (silenced unless it fails)
            subprocess.run([compressor, "-i", original, "-o", compressed],
                           check=True, capture_output=True, text=True)

            compressed += ".tzf"

            #decompression (silenced unless it fails)
            subprocess.run([compressor, "-i", compressed, "-o", restored],
                           check=True, capture_output=True, text=True)

        except subprocess.CalledProcessError as e:
            #ctf return anything different from 0, print its stdout/stderr so CI shows the error
            print(f"\n[ERROR] Subprocess failed with exit code {e.returncode}")
            if e.stdout: print(f"--- Subprocess stdout ---\n{e.stdout}")
            if e.stderr: print(f"--- Subprocess stderr ---\n{e.stderr}")
            print(f"Using RNG seed: {seed}")
            sys.exit(1)

        if not filecmp.cmp(original, restored, shallow=False):
            print(f"\n[ERROR] Validation failed: files differ on iteration {i + 1}")
            print(f"Using RNG seed: {seed}")

            offending_dir = os.path.join(os.path.expanduser("~"), "offenders")
            os.makedirs(offending_dir, exist_ok=True)

            #add prefix for iteration
            prefix = f"iter{i}_seed{seed}_"
            shutil.copy(original, os.path.join(offending_dir, prefix + "original.bin"))
            shutil.copy(compressed, os.path.join(offending_dir, prefix + "compressed.tzf"))
            shutil.copy(restored, os.path.join(offending_dir, prefix + "restored.bin"))

            print(f"Saved offending files in {offending_dir}")
            sys.exit(1)

print(f"All {ITERATIONS} iterations passed successfully.")