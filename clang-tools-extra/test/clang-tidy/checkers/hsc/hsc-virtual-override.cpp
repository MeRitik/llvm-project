// RUN: %check_clang_tidy %s hsc-virtual-override %t

class Parent {
public:
	virtual ~Parent() = default;
	virtual void method1() noexcept = 0;
	virtual void method2() noexcept {}
	void method4() noexcept {}
	virtual void method5() noexcept final = 0;
	// CHECK-MESSAGES: :[[@LINE-1]]:16: warning: use only one specifier: avoid combining 'virtual' and 'final' [hsc-virtual-override]
};

class Child : public Parent {
public:
	~Child();
	// CHECK-MESSAGES: :[[@LINE-1]]:3: warning: overriding virtual member function should use 'override' or 'final' [hsc-virtual-override]

	virtual void method1() noexcept override {}
	// CHECK-MESSAGES: :[[@LINE-1]]:16: warning: use only one specifier: avoid combining 'virtual' and 'override' [hsc-virtual-override]

	void method2() noexcept override final {}
	// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: use only one specifier: avoid combining 'override' and 'final' [hsc-virtual-override]

	void method5() noexcept {}
	// CHECK-MESSAGES: :[[@LINE-1]]:8: warning: overriding virtual member function should use 'override' or 'final' [hsc-virtual-override]
};

class ChildOk : public Parent {
public:
	~ChildOk() override;
	void method1() noexcept override {}
	void method2() noexcept final {}
	void method5() noexcept override {}
};
