package main

import "fmt"

//====================================

type OrangeObj interface {
	Print()
	Eval(Env) OrangeObj
}

/* primitive */
type OrangePrimitive func(OrangeObj) OrangeObj

func (obj OrangePrimitive) Print() {
	fmt.Printf("<#primitive>")
}

func (obj OrangePrimitive) Eval(env Env) OrangeObj {
	return Void
}

/* undefined */
type OrangeUndefined struct{}

func (obj OrangeUndefined) Print() {
	fmt.Printf("<#undefined>")
}

func (obj OrangeUndefined) Eval(env Env) OrangeObj {
	return Undefined
}

/* void */
type OrangeVoid struct{}

func (obj OrangeVoid) Print() {
	fmt.Printf("<#void>")
}

func (ovoid OrangeVoid) Eval(env Env) OrangeObj {
	return Void
}

/* boolean */
type OrangeBoolean bool

func (obj OrangeBoolean) Print() {
	var s string
	if obj {
		s = "#t"
	} else {
		s = "#f"
	}
	fmt.Printf("%s", s)
}
func (obj OrangeBoolean) Eval(env Env) OrangeObj {
	return obj
}

/* integer */
type OrangeInteger int

func (obj OrangeInteger) Print() {
	fmt.Printf("%d", obj)
}
func (obj OrangeInteger) Eval(env Env) OrangeObj {
	return obj
}

/* string */
type OrangeString string

func (obj OrangeString) Print() {
	fmt.Printf("%s", obj)
}
func (obj OrangeString) Eval(env Env) OrangeObj {
	return obj
}

/* symbol */
type OrangeSymbol string

func (obj OrangeSymbol) Print() {
	fmt.Printf("'%s", obj)
}
func (obj OrangeSymbol) Eval(env Env) OrangeObj {
	return Void
}

/* pair */
type OrangePair struct {
	car OrangeObj
	cdr OrangeObj
}

func (obj OrangePair) Print() {
	if obj == Nil {
		//fmt.Printf("'()")
		fmt.Printf("nil")
	} else {
		fmt.Printf("(")
		obj.car.Print()
		fmt.Printf("%c", ',')
		obj.cdr.Print()
		fmt.Printf(")")
	}
}

/* eval */
func (obj OrangePair) Eval(env Env) OrangeObj {
	if obj == Nil {
		return Nil
	} else {
		first := car(obj)
		if sym, ok := first.(OrangeSymbol); ok {
			switch sym {
			case "if":
				sym.Print()
			case "lambda":
				sym.Print()
			default:
				lookup_variable_value(sym, env)
			}
		} else if pair, ok := first.(OrangePair); ok {
			proc := car(pair).Eval(env)
			if proc, ok := proc.(OrangePair); ok {
				return apply(proc, list_of_values(pair, env))
			} else {
				panic("some thing error!")
			}
		} else {
			panic("not supported exp type")
		}
		return Void
	}
}

//====================================

type Env []map[OrangeSymbol]OrangeObj

/* get value for a variable */
func lookup_variable_value(sym OrangeSymbol, env Env) OrangeObj {
	return Void
}

/* eval parameters */
func list_of_values(pair OrangePair, env Env) OrangePair {
	return OrangePair{}
}

/*
('primitive <proc>)
('procedure params,body,env)
*/
func apply(procedure OrangePair, arguments OrangePair) OrangeObj {
	first := car(procedure)
	if sym, ok := first.(OrangeSymbol); ok {
		if sym == "primitive" {
		} else if sym == "procedure" {
		} else {
			panic("not supported procedure type")
		}
	} else {
		panic("error! first must be OrangeSymbol")
	}
	return Void
}

//====================================

/* global values */
var Nil OrangePair = OrangePair{}
var Undefined OrangeUndefined = OrangeUndefined{}
var Void OrangeVoid = OrangeVoid{}

//====================================

func cons(left OrangeObj, right OrangeObj) OrangePair {
	return OrangePair{left, right}
}

func car(obj OrangeObj) OrangeObj {
	if pair, ok := obj.(OrangePair); ok {
		return pair.car
	} else {
		panic("not pair object,can't get car")
	}
}
func cdr(obj OrangeObj) OrangeObj {
	if pair, ok := obj.(OrangePair); ok {
		return pair.cdr
	} else {
		panic("not pair object,can't get cdr")
	}
}

func cadr(pair OrangeObj) OrangeObj { return car(cdr(pair)) }
func cddr(pair OrangeObj) OrangeObj { return cdr(cdr(pair)) }

func caddr(pair OrangeObj) OrangeObj { return car(cdr(cdr(pair))) }
func caadr(pair OrangeObj) OrangeObj { return car(car(cdr(pair))) }
func cdadr(pair OrangeObj) OrangeObj { return cdr(car(cdr(pair))) }
func cdddr(pair OrangeObj) OrangeObj { return cdr(cdr(cdr(pair))) }

func cadddr(pair OrangeObj) OrangeObj { return car(cdr(cdr(cdr(pair)))) }

//====================================

func list(elements []OrangeObj) OrangePair {
	len := len(elements)
	if len == 0 {
		return Nil
	} else if len == 1 {
		return cons(elements[0], Nil)
	} else {
		var result_list OrangePair = Nil
		for i := len - 1; i > -1; i-- {
			result_list = cons(elements[i], result_list)
		}
		return result_list
	}
}

func test_list1() {
	nums := []int{1, 2, 3, 4, 5}
	var orange_nums []OrangeObj = []OrangeObj{}
	for i := 0; i < len(nums); i++ {
		orange_nums = append(orange_nums, OrangeInteger(nums[i]))
	}
	mylist := list(orange_nums)
	mylist.Print()
	fmt.Println()
}

//====================================

func main() {
	var left1 OrangeInteger = OrangeInteger(3)
	car(left1)
	var right1 OrangeInteger = 4
	//var n9 OrangeInteger = 4
	cell := cons(left1, cons(right1, Nil))
	cell.Print()
	fmt.Println()
	test_list1()
}
