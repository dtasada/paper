def heron(a,b,c):
    s = (a + b + c) / 2
    area = (s*(s-a) * (s-b)*(s-c)) ** 0.5
    return area


def distance3d(x1,y1,z1,x2,y2,z2):
    a=(x1-x2)**2+(y1-y2)**2 + (z1-z2)**2
    d= a ** 0.5
    return d


def areatriangle3d(x1,y1,z1,x2,y2,z2,x3,y3,z3):
    a=distance3d(x1,y1,z1,x2,y2,z2)
    b=distance3d(x2,y2,z2,x3,y3,z3)
    c=distance3d(x3,y3,z3,x1,y1,z1)
    A = heron(a,b,c)
    print("area of triangle is %r " %A)
