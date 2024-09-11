# paper

* To compile for windows: `CGO_ENABLED=1 CC=x86_64-w64-mingw32-gcc GOOS=windows GOARCH=amd64 go build -ldflags "-s -w"`
* Compile for darwin-amd64: `CGO_ENABLED=1 CC=x86_64-apple-darwin21.1-clang GOOS=darwin GOARCH=amd64 go build -ldflags "-linkmode external -s -w '-extldflags=-mmacosx-version-min=10.15'`
* Compile for darwin-arm64: `CGO_ENABLED=1 CC=aarch64-apple-darwin21.1-clang GOOS=darwin GOARCH=arm64 go build -ldflags "-linkmode external -s -w '-extldflags=-mmacosx-version-min=12.0.0'"`

## Deadlines
    - December 2: Hand in concept
    - February 2: Hand in final
    - February 13: Present

## Todo
* Make grid checking multithreaded

## Documentation:
(Wikipedia)[https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional]
(hakenberg.de)[http://www.hakenberg.de/diffgeo/collision_resolution.htm]
