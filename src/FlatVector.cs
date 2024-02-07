namespace FlatPhysics;

public readonly struct FlatVector {
	public readonly float x;
	public readonly float y;

	public static readonly FlatVector Zero = new FlatVector(0f, 0f);


	public override int GetHashCode() {
		return new { this.x, this.y }.GetHashCode();
	}

	public override string ToString() {
		return $"x: {this.x}, y: {this.y}";
	}

	public FlatVector(float x, float y) {
		this.x = x;
		this.y = y;
	}

	public static FlatVector operator +(FlatVector a, FlatVector b) {
		return new FlatVector(a.x + b.x, a.y + b.y);
	}

	public static FlatVector operator -(FlatVector a, FlatVector b) {
		return new FlatVector(a.x - b.x, a.y - b.y);
	}

	public static FlatVector operator -(FlatVector v) {
		return new FlatVector(-v.x, -v.y);
	}

	public static FlatVector operator *(FlatVector a, float s) {
		return new FlatVector(a.x * s, a.y * s);
	}

	public static FlatVector operator /(FlatVector a, float s) {
		return new FlatVector(a.x / s, a.y / s);
	}

	public static bool operator ==(FlatVector a, FlatVector b) {
		return a.x == b.x && a.y == b.y;
	}

	public static bool operator !=(FlatVector a, FlatVector b) {
		return a.x != b.x && a.y != b.y;
	}
}
