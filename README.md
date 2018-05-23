# briGITte
Git viewer inspired by gitg

# Installation

1. Get the sources, e.g.:
```
git clone https://github.com/cppdreams/brigitte.git
```

2. Build the project:
```
mkdir build
cd build
cmake ../brigitte
make -j 8
```

3. Set up config.yaml, e.g.:
```
cp ../brigitte/config.yaml.template config.yaml
# Now edit config.yaml, listing projects you want to view
```

4. Run!
```
./brigitte
```
