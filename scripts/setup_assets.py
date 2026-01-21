import os
import urllib.request
import ssl

# Define paths
ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) # go one up from scripts/
ASSETS_DIR = os.path.join(ROOT_DIR, "assets")
AUDIO_DIR = os.path.join(ASSETS_DIR, "audio")
LEVELS_DIR = os.path.join(ASSETS_DIR, "levels")

# Create directories
os.makedirs(ASSETS_DIR, exist_ok=True)
os.makedirs(AUDIO_DIR, exist_ok=True)
os.makedirs(LEVELS_DIR, exist_ok=True)

print(f"Setting up assets in: {ASSETS_DIR}")

# Bypass SSL errors for some older python envs
ssl._create_default_https_context = ssl._create_unverified_context

# ---------------------------------------------------------
# 1. DOWNLOAD TEXTURES
# ---------------------------------------------------------
# Map: Local Filename -> URL
textures = {
    "background.jpg": "https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/background.jpg",
    "block.png":      "https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/block.png",
    "block_solid.png":"https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/block_solid.png",
    "paddle.png":     "https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/paddle.png",
    "face.png":       "https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/awesomeface.png", 
    # Using a simple white circle for particle if not available, but let's try to fetch a particle image or reuse face scaled down
    "particle.png":   "https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/resources/textures/awesomeface.png" # Reusing face as placeholder for particle
}

for filename, url in textures.items():
    filepath = os.path.join(ASSETS_DIR, filename)
    if not os.path.exists(filepath):
        print(f"Downloading {filename}...")
        try:
            urllib.request.urlretrieve(url, filepath)
        except Exception as e:
            print(f"Failed to download {filename}: {e}")
    else:
        print(f" - {filename} exists.")

# ---------------------------------------------------------
# 2. DOWNLOAD AUDIO (Placeholders)
# ---------------------------------------------------------
# Using small wav files from a public repo or placeholders
audio_files = {
    "bleep.wav": "https://learnopengl.com/audio/breakout/bleep.wav",
    "pop.wav":   "https://learnopengl.com/audio/breakout/solid.wav"
}

for filename, url in audio_files.items():
    filepath = os.path.join(AUDIO_DIR, filename)
    if not os.path.exists(filepath):
        print(f"Downloading {filename}...")
        try:
            urllib.request.urlretrieve(url, filepath)
        except Exception as e:
            print(f"Failed to download {filename}: {e}")
    else:
        print(f" - {filename} exists.")

# ---------------------------------------------------------
# 3. CREATE LEVEL FILES
# ---------------------------------------------------------
levels = {
    "one.lvl": """
5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
4 4 4 4 4 4 4 4 4 4 4 4 4 4 4
4 4 4 4 4 4 4 4 4 4 4 4 4 4 4
3 3 3 3 3 3 3 3 3 3 3 3 3 3 3
3 3 3 3 3 3 3 3 3 3 3 3 3 3 3
2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
""".strip(),

    "two.lvl": """
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
1 0 0 0 0 0 0 0 0 0 0 0 0 0 1
1 0 2 2 2 0 0 0 0 0 2 2 2 0 1
1 0 2 5 2 0 0 0 0 0 2 5 2 0 1
1 0 2 2 2 0 0 0 0 0 2 2 2 0 1
1 0 0 0 0 0 0 3 0 0 0 0 0 0 1
1 0 0 0 0 3 3 3 3 3 0 0 0 0 1
1 0 4 0 0 0 0 0 0 0 0 0 4 0 1
1 0 4 4 4 4 4 4 4 4 4 4 4 0 1
1 0 0 0 0 0 0 0 0 0 0 0 0 0 1
""".strip(),

    "three.lvl": """
5 0 0 0 0 0 0 5 0 0 0 0 0 0 5
0 5 0 0 0 0 0 5 0 0 0 0 0 5 0
0 0 5 0 0 0 0 5 0 0 0 0 5 0 0
0 0 0 5 0 0 0 5 0 0 0 5 0 0 0
0 0 0 0 5 0 0 5 0 0 5 0 0 0 0
1 1 1 1 0 5 0 5 0 5 0 1 1 1 1
0 0 0 0 0 0 5 5 5 0 0 0 0 0 0
0 0 0 0 0 0 0 5 0 0 0 0 0 0 0
2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
""".strip(),

    "four.lvl": """
1 1 1 1 1 1 1 0 1 1 1 1 1 1 1
4 0 4 4 0 4 4 0 4 4 0 4 4 0 4
3 3 0 3 0 3 3 0 3 3 0 3 0 3 3
2 2 0 0 0 2 2 1 2 2 0 0 0 2 2
5 5 5 5 5 5 5 0 5 5 5 5 5 5 5
0 2 2 2 2 2 0 0 0 2 2 2 2 2 0
0 3 1 3 1 3 0 0 0 3 1 3 1 3 0
0 4 4 4 4 4 0 0 0 4 4 4 4 4 0
0 5 0 0 0 5 0 0 0 5 0 0 0 5 0
""".strip()
}

for filename, content in levels.items():
    filepath = os.path.join(LEVELS_DIR, filename)
    with open(filepath, "w") as f:
        f.write(content)
    print(f"Created level: {filename}")

print("\nAsset Setup Complete!")
