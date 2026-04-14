/*
	Singleton

	싱글톤 패턴 : 단일 객체 관리
	- 인스턴스를 호출 시 처음 호출(인스턴스가 생성 되기 전)이 되면 인스턴스를 생성해서 반환한다.
	- 인스턴스가 생성 된 이후 호출 하게 되면 기존에 생성 된 인스턴스를 반환한다.

	템플릿(Template, 형판) : 함수 또는 클래스를 만들어내기 위한 틀
	- 정해지지 않은 자료형에 대한 선언을 템플릿을 사용하여 하고 기능들을 구현한다.
	- 함수 또는 클래스 작성시 템플릿에 사용 될 자료형을 결정해준다.
*/

#pragma once
template <typename T> // template<class T>
class singleton
{
protected:
	singleton() = default;
	~singleton() = default;

	singleton(const singleton&) = delete;
	singleton& operator=(const singleton&) = delete;

public:
	static T& Get() {
		static T instance;
		return instance;
	}
};