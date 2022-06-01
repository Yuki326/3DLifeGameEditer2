
# include <Siv3D.hpp> // OpenSiv3D v0.6.3

struct AfinParameter3D {
	double a;
	double b;
	double c;
	double d;
	double e;
	double f;
	double g;
	double h;
	double i;
	double j;
	double k;
	double l;
	double m;
	double n;
	double o;
	double p;
};
struct Angle {
	double w;
	double h;
};
struct _Vec2 {
	double x;
	double y;
};
struct _Vec3 {
	double x;
	double y;
	double z;
};
struct Object {
	Angle angle;
	_Vec3 pos;
};
struct _Triangle2D {
	_Vec2 p0;
	_Vec2 p1;
	_Vec2 p2;
};
struct _Triangle3D {
	_Vec3 p0;
	_Vec3 p1;
	_Vec3 p2;
};

struct _Polygon3D {
	_Triangle3D points;
	Color color;
};
struct _Polygon {
	Triangle points;
	Color color;
};
struct _Model {
	Array<_Polygon3D> shape;
	Object object;
	_Vec3 zahyo; //ゲームのフィールド格子)上の座標
	int hp;
};
struct LifeGameInfo {
	Array<_Model> model;
	Grid<int32> fieldstate;
};
AfinParameter3D viewingPiperine;
const double CELL_PER = 0.01;
const int SIDE_CELLS = 40;
// 共通
_Vec3 changePos3D(_Vec3 p, AfinParameter3D afin) {
	_Vec3 res;
	res.x = afin.a * p.x + afin.b * p.y + afin.c * p.z + afin.d;
	res.y = afin.e * p.x + afin.f * p.y + afin.g * p.z + afin.h;
	res.z = afin.i * p.x + afin.j * p.y + afin.k * p.z + afin.l;
	return res;
}
_Polygon3D transFormTriangle3D(_Polygon3D t, AfinParameter3D afin) {
	t.points.p0 = changePos3D(t.points.p0, afin);
	t.points.p1 = changePos3D(t.points.p1, afin);
	t.points.p2 = changePos3D(t.points.p2, afin);
	return t;
}
Array<_Polygon3D> transFormModel(Array<_Polygon3D> triangles, AfinParameter3D afin) {
	return triangles.map([afin](_Polygon3D t) { return transFormTriangle3D(t, afin); });
}
Array<_Model> transFormModels(Array<_Model> models, AfinParameter3D afin) {
	for (int i = 0; i < models.size(); i++) {
		models[i].shape = transFormModel(models[i].shape, afin);
	}
	return models;
}
AfinParameter3D combineAfin(AfinParameter3D x, AfinParameter3D y) {
	AfinParameter3D res;
	res.a = x.a * y.a + x.e * y.b + x.i * y.c + x.m * y.d;
	res.b = x.b * y.a + x.f * y.b + x.j * y.c + x.n * y.d;
	res.c = x.c * y.a + x.g * y.b + x.k * y.c + x.o * y.d;
	res.d = x.d * y.a + x.h * y.b + x.l * y.c + x.p * y.d;
	res.e = x.a * y.e + x.e * y.f + x.i * y.g + x.m * y.h;
	res.f = x.b * y.e + x.f * y.f + x.j * y.g + x.n * y.h;
	res.g = x.c * y.e + x.g * y.f + x.k * y.g + x.o * y.h;
	res.h = x.d * y.e + x.h * y.f + x.l * y.g + x.p * y.h;
	res.i = x.a * y.i + x.e * y.j + x.i * y.k + x.m * y.l;
	res.j = x.b * y.i + x.f * y.j + x.j * y.k + x.n * y.l;
	res.k = x.c * y.i + x.g * y.j + x.k * y.k + x.o * y.l;
	res.l = x.d * y.i + x.h * y.j + x.l * y.k + x.p * y.l;
	res.m = x.a * y.m + x.e * y.n + x.i * y.o + x.m * y.p;
	res.n = x.b * y.m + x.f * y.n + x.j * y.o + x.n * y.p;
	res.o = x.c * y.m + x.g * y.n + x.k * y.o + x.o * y.p;
	res.p = x.d * y.m + x.h * y.n + x.l * y.o + x.p * y.p;
	return res;
}

//-----
//ポリゴンの表裏判定
//-------
_Vec3 cross_product(const _Vec3 vl, const _Vec3 vr)
{
	_Vec3 ret;
	ret.x = vl.y * vr.z - vl.z * vr.y;
	ret.y = vl.z * vr.x - vl.x * vr.z;
	ret.z = vl.x * vr.y - vl.y * vr.x;

	return ret;
}

//ベクトル内積
double dot_product(const _Vec3 vl, const _Vec3 vr) {
	return vl.x * vr.x + vl.y * vr.y + vl.z * vr.z;
}

// ベクトルvに対してポリゴンが表裏どちらを向くかを求める
// 戻り値    0:表    1:裏    -1:エラー
int polygon_side_chk(_Triangle3D t, _Vec3 v) {

	//ABCが三角形かどうか。ベクトルvが0でないかの判定は省略します
	_Vec3 A = t.p0;
	_Vec3 B = t.p1;
	_Vec3 C = t.p2;
	//AB BCベクトル
	_Vec3 AB;
	_Vec3 BC;

	AB.x = B.x - A.x;
	AB.y = B.y - A.y;
	AB.z = B.z - A.z;

	BC.x = C.x - A.x;
	BC.y = C.y - A.y;
	BC.z = C.z - A.z;

	//AB BCの外積
	_Vec3 c = cross_product(AB, BC);
	double dist = t.p0.z + t.p1.z + t.p2.z;
	if (dist < 3) {
		return 0;
	}
	//ベクトルvと内積。順、逆方向かどうか調べる
	double d = dot_product(v, c);

	if (d < 0.0) {
		return 1;    //ポリゴンはベクトルvから見て表側
	}
	return 0;
}
bool isFartherTriangle(_Polygon3D t, _Polygon3D a) {
	double targetDist = t.points.p0.z + t.points.p1.z + t.points.p2.z;
	double dist = a.points.p0.z + a.points.p1.z + a.points.p2.z;
	return targetDist > dist;
}
Array<_Polygon3D> sortTriangle3D(Array<_Polygon3D> triangles) {//奥行ソート
	for (int i = 0; i < triangles.size(); i++) {//todo 速いソートに変更
		for (int j = i; j < triangles.size(); j++) {
			if (isFartherTriangle(triangles[i], triangles[j])) {
				_Polygon3D tmp = triangles[i];
				triangles[i] = triangles[j];
				triangles[j] = tmp;
			}
		}
	}
	return triangles;
}
// 投影変換
Vec2 toVec2(_Vec3 pos) {
	return Vec2{ pos.x,pos.y };
	//return Vec2{ pos.x/pos.z*200,pos.y/pos.z*200 };//投視投影　現時点だと歪んで見える
}
_Polygon renderTriangle(_Polygon3D t) {
	_Polygon result;
	result.points.p0 = toVec2(t.points.p0);
	result.points.p1 = toVec2(t.points.p1);
	result.points.p2 = toVec2(t.points.p2);
	result.color = t.color;

	return result;
}
Array<_Polygon> renderModel(Array<_Polygon3D> triangles) {
	_Polygon n = {};
	triangles = sortTriangle3D(triangles);

	return triangles.map([n](_Polygon3D t) { return polygon_side_chk(t.points, _Vec3{ 0,0,1 }) ? renderTriangle(t) : n; });
}
Array<_Polygon> render(Array<_Model> models) {
	Array<_Polygon> res = {};
	for (int i = 0; i < models.size(); i++) {
		Array<_Polygon> toAdd = renderModel(models[i].shape);
		for (int j = 0; j < toAdd.size(); j++) {
			res << toAdd[j];
		}
	}
	return res;
	//Array<_Triangle3D> all;//ポリゴン数が増えるとソートに時間がかかるため一旦コメントアウト
	//Array<_Triangle3D> toAdd;
	//for (int i = 0; i < models.size(); i++) {
		//toAdd = models[i].shape;
		//for (int j = 0; j < toAdd.size(); j++) {
			//all << toAdd[j];
		//}
	//}
	//return renderModel(all);
}
//ビューポート変換
Vec2 moveCenterPos(Vec2 p) {
	return p + Scene::Center();
}
_Polygon moveCenterTriangle(_Polygon t) {
	t.points.p0 = moveCenterPos(t.points.p0);
	t.points.p1 = moveCenterPos(t.points.p1);
	t.points.p2 = moveCenterPos(t.points.p2);
	return t;
}
Array<_Polygon> moveCenterModel(Array<_Polygon> triangles) {
	return triangles.map([](_Polygon t) { return moveCenterTriangle(t); });
}

//モデリング変換
Array<_Polygon3D> toWorldModel(Array<_Polygon3D> triangles, Object object) {
	AfinParameter3D afin1, afin2, afin3;
	double w = object.angle.w / 50;
	double h = object.angle.h / 50;
	afin1 = { cos(w),0,-sin(w),0,0,1,0,0,sin(w),0,cos(w),0 };
	afin2 = { 1,0,0,0,
		0,cos(h),sin(h),0,
		0,-sin(h),cos(h),0 };
	triangles = transFormModel(triangles, combineAfin(afin2, afin1));
	viewingPiperine = combineAfin(afin2, afin1);
	_Vec3 p = object.pos;
	afin3 = { 1,0,0,p.x,0,1,0,p.y,0,0,1,p.z };
	triangles = transFormModel(triangles, afin3);
	return triangles;
}
Array<_Model> toWorld(Array<_Model> models) {
	for (int i = 0; i < models.size(); i++) {
		models[i].shape = toWorldModel(models[i].shape, models[i].object);
	}
	return models;
}
// 視野変換

Array<_Polygon3D> conversionFieldModel(Array<_Polygon3D> triangles, Object camera) {
	AfinParameter3D afin1, afin2, afin3, afin4;

	_Vec3 p = camera.pos;
	afin3 = { 1,0,0,-p.x,0,1,0,-p.y,0,0,1,-p.z };
	triangles = transFormModel(triangles, afin3);
	double w = camera.angle.w / 50;
	double h = camera.angle.h / 50;
	afin1 = { cos(w),0,-sin(w),0,0,1,0,0,sin(w),0,cos(w),0 };
	afin2 = { 1,0,0,0,0,cos(h),sin(h),0,0,-sin(h),cos(h),0 };
	triangles = transFormModel(triangles, combineAfin(afin2, afin1));

	return triangles;
}
Array<_Model> conversionField(Array<_Model> models, Object camera) {
	for (int i = 0; i < models.size(); i++) {
		models[i].shape = toWorldModel(models[i].shape, camera);
	}
	return models;
}
Array<_Polygon3D> resizeModel(Array<_Polygon3D> model, double rate) {
	AfinParameter3D afin = { rate,0,0,0,0,rate,0,0,0,0,rate,0,0,0,0,1 };
	return transFormModel(model, afin);
}
Array<_Polygon3D> paintModel(Array<_Polygon3D> model, Color c) {
	for (int i = 0; i < model.size(); i++) {
		model[i].color = c;
	}
	return model;
}
Array<_Polygon3D> putModel(Array<_Polygon3D> models, _Vec3 pos) {
	AfinParameter3D afin = { 1,0,0,pos.x,0,1,0,pos.y,0,0,1,pos.z,0,0,0,1 };
	return transFormModel(models, afin);
}
LifeGameInfo setLifeGame(Array<_Polygon3D> cubePolygons, Object core) {
	_Vec3 pos;
	Grid<int32> fieldState(SIDE_CELLS, SIDE_CELLS, 0);
	Array<_Polygon3D> framePolygons = resizeModel(cubePolygons, 60);
	framePolygons = paintModel(framePolygons, { 0,255,0,45 });
	Array<_Polygon3D> framePolygons2 = resizeModel(framePolygons, 0.1);
	Array<_Model> models = {
		{framePolygons,core,{0,0,0},100},
	};
	for (int i = 0; i < SIDE_CELLS; i++) {
		pos.x = 4 * (i - SIDE_CELLS / 2);
		for (int j = 0; j < SIDE_CELLS; j++) {
			pos.y = 4 * (j - SIDE_CELLS / 2);
			for (int k = 0; k < SIDE_CELLS; k++) {
				pos.z = 4 * (k - SIDE_CELLS / 2);
				if (rand() % 10000 <= CELL_PER * 100) {
					models << _Model{ putModel(cubePolygons,pos), core, { i,j,k }, 100 };
					fieldState[i][j] &= 1 << k;
				}
			}
		}
	}
	return LifeGameInfo{ models,fieldState };
}
void Main()
{
	// 背景を黒にする
	Scene::SetBackground(Palette::Black);

	// 大きさ 60 のフォントを用意
	const Font font(60);

	//モデリング
	Array<_Vec3> samplePoints = {
		//{0,0,0},{200,0,0},{200,200,0},{0,200,0},
		{-20,20,-20},{0,200,0},{200,0,0},
		{0,0,200},{300,300,300},{0,0,100},{0,100,0},
		{100,80,100},{0,80,100}
	};
	Array<_Polygon3D> samplePolygons = {
		{_Triangle3D{ samplePoints[0], samplePoints[2], samplePoints[1] },Color{255,0,0}},
		{_Triangle3D{ samplePoints[0], samplePoints[1], samplePoints[3] },Color{100,255,0}},
		{_Triangle3D{ samplePoints[0], samplePoints[3], samplePoints[2] },Color{100,0,0}},
		{_Triangle3D{ samplePoints[4], samplePoints[1], samplePoints[2] },Color{100,0,255}},
		{_Triangle3D{ samplePoints[4], samplePoints[3], samplePoints[1] },Color{0,0,100}},
		{_Triangle3D{ samplePoints[4], samplePoints[2], samplePoints[3] },Color{100,100,100}},
	};
	Array<_Vec3> cubePoints = {
	{-2,-2,-2},{2,-2,-2},{2,-2,2},{-2,-2,2},
	{-2,2,-2},{2,2,-2},{2,2,2},{-2,2,2}
	};
	Array<_Polygon3D> cubePolygons = {
	{_Triangle3D{ cubePoints[0], cubePoints[3], cubePoints[1] },Color{0,255,0}},
	{_Triangle3D{ cubePoints[1], cubePoints[3], cubePoints[2] },Color{0,255,0}},
	{_Triangle3D{ cubePoints[4], cubePoints[5], cubePoints[7] },Color{255,0,0}},
	{_Triangle3D{ cubePoints[5], cubePoints[6], cubePoints[7] },Color{255,0,0}},
	{_Triangle3D{ cubePoints[0], cubePoints[5], cubePoints[4] },Color{0,0,255}},
	{_Triangle3D{ cubePoints[1], cubePoints[5], cubePoints[0] },Color{0,0,255}},
	{_Triangle3D{ cubePoints[0], cubePoints[4], cubePoints[7] },Color{0,255,255}},
	{_Triangle3D{ cubePoints[3], cubePoints[0], cubePoints[7] },Color{0,255,255}},

	{_Triangle3D{ cubePoints[2], cubePoints[7], cubePoints[6] },Color{255,255,0}},
	{_Triangle3D{ cubePoints[3], cubePoints[7], cubePoints[2] },Color{255,255,0}},

	{_Triangle3D{ cubePoints[2], cubePoints[6], cubePoints[5] },Color{255,0,255}},
	{_Triangle3D{ cubePoints[1], cubePoints[2], cubePoints[5] },Color{255,0,255}},

	};
	Object ex0 = { Angle{0,0},_Vec3{0,0,0} };
	Object core = { Angle{0,0},_Vec3{0,0,500} };
	Object ex2 = { Angle{0,-12},_Vec3{0,-40,500} };
	Object ex3 = { Angle{0,-12},_Vec3{50,50,50} };

	LifeGameInfo data = setLifeGame(cubePolygons, core);
	Array<_Model> models = data.model;
	Grid<int32> fieldState(SIDE_CELLS, SIDE_CELLS, 0);//３次元配列　z軸は2進数で管理
	fieldState = data.fieldstate;


	//モデリング変換
	Array<_Model> models_W = toWorld(models);
	Object camera = { Angle{0,10},_Vec3{0,0,0} };

	while (System::Update())
	{
		const double delta = 200 * Scene::DeltaTime();

		// 上下左右キーで移動
		if (KeyA.pressed())
		{
			models[1].object.pos.x += delta;
		}

		if (KeyD.pressed())
		{
			models[1].object.pos.x -= delta;
		}

		if (KeyW.pressed())
		{
			models[1].object.pos.z -= delta;
		}

		if (KeyS.pressed())
		{
			models[1].object.pos.z += delta;
		}
		if (KeySpace.pressed())
		{
			models[1].object.pos.y += delta;
		}

		if (KeyShift.pressed())
		{
			models[1].object.pos.y -= delta;
		}
		if (SimpleGUI::Button(U"Reset", Vec2(600, 300), 200))
		{
			LifeGameInfo data = setLifeGame(cubePolygons, core);
			models = data.model;
			fieldState = data.fieldstate;
		}

		ClearPrint();
		for (int i = 0; i < models.size(); i++) {
			models[i].object.angle.w += delta / 3;
		}
		const double hue = Scene::Time() * 60.0;
		models[0].shape[4].color = HSV(hue, 0.6, 1.0);

		//視点移動
		//camera.angle.w = Cursor::Pos().x - Scene::Center().x;
		//camera.angle.h = Cursor::Pos().y - Scene::Center().y;

		//モデリング変換
		models_W = toWorld(models);

		//視野変換
		Array<_Model> models_W_camera = conversionField(models_W, camera);

		// 投影変換
		Array<_Polygon> t = render(models_W_camera);

		// ビューポート変換
		t = moveCenterModel(t);

		//描画
		t.map([](_Polygon t) {t.points.draw(t.color);  return 0; });


		//デバッグ
		Print << Cursor::Pos(); // 現在のマウスカーソル座標を表示
		Print << camera.angle.w;
	}
}

//参考　http://www.sousakuba.com/Programming/gs_polygon_inside_outside.html ポリゴン表裏判定

//
// = アドバイス =
// Debug ビルドではプログラムの最適化がオフになります。
// 実行速度が遅いと感じた場合は Release ビルドを試しましょう。
// アプリをリリースするときにも、Release ビルドにするのを忘れないように！
//
// 思ったように動作しない場合は「デバッグの開始」でプログラムを実行すると、
// 出力ウィンドウに詳細なログが表示されるので、エラーの原因を見つけやすくなります。
//
// = お役立ちリンク =
//
// OpenSiv3D リファレンス
// https://siv3d.github.io/ja-jp/
//
// チュートリアル
// https://siv3d.github.io/ja-jp/tutorial/basic/
//
// よくある間違い
// https://siv3d.github.io/ja-jp/articles/mistakes/
//
// サポートについて
// https://siv3d.github.io/ja-jp/support/support/
//
// Siv3D ユーザコミュニティ Slack への参加
// https://siv3d.github.io/ja-jp/community/community/
//
// 新機能の提案やバグの報告
// https://github.com/Siv3D/OpenSiv3D/issues
//
