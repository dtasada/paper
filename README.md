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

## Variables
    - Mass _m_:             R^+
    - Position _p_:         R^3
    - Orientation _R_:      SO(3)
    - Velocity _v_:         R^3
    - Angular velocity _ω_: R^3

### Inertia (Wikipedia)[https://en.wikipedia.org/wiki/Moment_of_inertia]
    - Inertia tensor _I_:   _I_ = L / ω  where L is angular momentum
    - Unit:                 kg*m^2

### Angular momentum (Wikipedia)[https://en.wikipedia.org/wiki/Angular_momentum]
    - Angular momentum:     _L_ = mvr
    - Unit:                kg*m^2/s

### Angular velocity (Wikipedia)[https://en.wikipedia.org/wiki/Angular_velocity]
    - Angular velocity:     ω = dθ / dt
    - Unit:                 rad/s

## Post-collision formulas:
    - m1v1
