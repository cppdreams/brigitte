# briGITte
Git viewer inspired by gitg

# Installation

1. Get the sources, e.g.:

```
git clone https://github.com/cppdreams/brigitte.git
```

2. Navigate to the source directory, then:

```
cd ..
mkdir build
cd build
cmake ../brigitte
make -j 8
```

3. Set up config.yaml, e.g.:

```
cp ../brigitte/config.yaml .
# Now edit config.yaml, listing projects you want to view
```

4. Run!

```
./brigitte
```
