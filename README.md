# Sponge Lab

## About

This is a programming lab of CSED353 in POSTECH.  
This lab is about implementing simple Internet comprises 5 layers
(but, does not implement Physical Layer).  
To test the code, reference `README.md` in `/setup`.
It would tell how to compose environment and several tips to setup.

## Test

1. Clone the repo

```
git clone https://github.com/moonsong98/CSED353.git
```

2. Move in to the directory and make `build` directory

```
cd CSED353 &&
mkdir build &&
cd $_
```

3. run cmake

```
cmake ../
```

4. run make

```
make
```

5. run tests

```
make check
```
