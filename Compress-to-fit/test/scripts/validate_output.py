import subprocess, tempfile, filecmp, os, sys, random

compressor = sys.argv[1]

seed = random.randrange(sys.maxsize)
random.seed(seed)

with tempfile.TemporaryDirectory() as temp_dir:
    original = os.path.join(temp_dir, "original.bin")
    with open(original, "wb") as dummy:
        dummy.write(random.randbytes(1024 * 1024))#1MB

    compressed = os.path.join(temp_dir, "compressed")
    restored = os.path.join(temp_dir, "restored")

    subprocess.run([compressor, "-i", original, "-o", compressed], check=True)
    print("Ran compression")
    compressed += ".tzf"
    subprocess.run([compressor, "-i", compressed, "-o", restored], check=True)
    print("Ran decompression")

    print(f"Using RNG seed: {seed}")

    if not filecmp.cmp(original, restored, shallow=False):
        print("Validation failed: files differ")
        sys.exit(1)


