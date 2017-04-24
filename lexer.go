package main

import "fmt"
import "strings"
import "log"
import "bufio"
import "io"
import "os"

const (
	Lparen = 1
	Rparen = 2
	Quote  = 3
	Number = 4
	String = 5
	Symbol = 6
)

type Token struct {
	ty      int
	literal string
}

type Lexer struct {
	tokens    []Token
	receivers []RuneReceiver
}

func (lexer *Lexer) Init() {
	lexer.AppendReceiver(new(SingleBucket))
	lexer.AppendReceiver(new(WhitespaceBucket))
	lexer.AppendReceiver(new(NumberBucket))
	lexer.AppendReceiver(new(StringBucket))
	lexer.AppendReceiver(new(SymbolBucket))

	for _, r := range lexer.receivers {
		r.SetLexer(lexer)
	}
}

func (lexer *Lexer) IsAnyOpen() bool {
	for _, r := range lexer.receivers {
		if r.IsOpen() {
			return true
		}
	}
	return false
}

func (lexer *Lexer) AppendReceiver(r RuneReceiver) {
	lexer.receivers = append(lexer.receivers, r)
}

func (lexer *Lexer) AppendToken(t Token) {
	lexer.tokens = append(lexer.tokens, t)
}

func (lexer *Lexer) TailClean() {
	for _, r := range lexer.receivers {
		if r.IsOpen() {
			r.Close()
		}
	}
}

type RuneReceiver interface {
	Take(c rune) (isAccepted bool)
	IsOpen() bool
	Open()
	Close()
	SetLexer(lexer *Lexer)
}

type Bucket struct {
	lexer   *Lexer
	ty      int
	is_open bool
	buf     []rune
}

func (bucket *Bucket) BufLen() int   { return len(bucket.buf) }
func (bucket *Bucket) Append(c rune) { bucket.buf = append(bucket.buf, c) }
func (bucket *Bucket) Close() {
	if bucket.BufLen() > 0 {
		bucket.lexer.AppendToken(Token{bucket.ty, bucket.String()})
	}
	bucket.buf = []rune{}
	bucket.is_open = false
}
func (bucket *Bucket) Open()                 { bucket.is_open = true }
func (bucket *Bucket) IsOpen() bool          { return bucket.is_open }
func (bucket *Bucket) String() string        { return string(bucket.buf) }
func (bucket *Bucket) SetLexer(lexer *Lexer) { bucket.lexer = lexer }

type SingleBucket struct{ Bucket }
type WhitespaceBucket struct{ Bucket }
type NumberBucket struct{ Bucket }
type StringBucket struct{ Bucket }
type SymbolBucket struct{ Bucket }

func (bucket *SingleBucket) Take(c rune) (isAccepted bool) {
	//log.Printf("single: %c\n", c)
	s := "()'"
	if contains(s, c) {
		switch c {
		case '(':
			bucket.ty = Lparen
			bucket.Append(c)
		case ')':
			bucket.ty = Rparen
			bucket.Append(c)
		case '\'':
			bucket.ty = Quote
			bucket.Append(c)
		}
		bucket.Close()
		isAccepted = true
	} else {
		bucket.Close()
		isAccepted = false
	}
	return
}

func (bucket *WhitespaceBucket) Take(c rune) (isAccepted bool) {
	//log.Printf("whitespace: %c\n", c)
	s := " \n\t\r\v\f"
	if !contains(s, c) {
		bucket.Close()
		isAccepted = false
	} else {
		isAccepted = true
	}
	return
}

func (bucket *NumberBucket) Take(c rune) (isAccepted bool) {
	//log.Printf("number: %c\n", c)
	s := "0123456789"
	if contains(s, c) {
		bucket.Append(c)
		isAccepted = true
	} else {
		bucket.ty = Number
		bucket.Close()
		isAccepted = false
	}
	return
}

func (bucket *StringBucket) Take(c rune) (isAccepted bool) {
	//log.Printf("string: %c\n", c)
	if c == '"' {
		if bucket.BufLen() > 0 {
			bucket.Append(c)
			bucket.ty = String
			bucket.Close()
			isAccepted = true
		} else {
			bucket.Append(c)
			isAccepted = true
		}
	} else {
		if bucket.BufLen() > 0 {
			bucket.Append(c)
			isAccepted = true
		} else {
			bucket.ty = String
			bucket.Close()
			isAccepted = false
		}
	}
	return
}

func (bucket *SymbolBucket) Take(c rune) (isAccepted bool) {
	//log.Printf("symbol: %c\n", c)
	letter_s := "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
	special_s := "!$%&*/:<=>?^_~"
	initial_s := letter_s + special_s
	special_sub_s := "+-.@"
	digit_s := "0123456789"
	subsequent_s := initial_s + digit_s + special_sub_s

	is_peculiar := false

	if bucket.BufLen() == 0 {
		if c == '+' || c == '-' {
			bucket.Append(c)
			bucket.ty = Symbol
			bucket.Close()
			isAccepted = true
		} else if c == '.' {
			bucket.Append(c)
			is_peculiar = true
			isAccepted = true
		} else {
			if contains(initial_s, c) {
				bucket.Append(c)
				isAccepted = true
			} else {
				bucket.ty = Symbol
				bucket.Close()
				isAccepted = false
			}
		}
	} else {
		if is_peculiar {
			if c == '.' && bucket.BufLen() < 3 {
				bucket.Append(c)
				isAccepted = true
			} else {
				bucket.ty = Symbol
				bucket.Close()
				isAccepted = false
			}

		} else {
			if contains(subsequent_s, c) {
				bucket.Append(c)
				isAccepted = true
			} else {
				bucket.ty = Symbol
				bucket.Close()
				isAccepted = false
			}
		}
	}
	return
}

func contains(s string, r rune) bool { return strings.IndexRune(s, r) != -1 }

func read(file_name string) {
	log.Println("----read file----")

	lexer := new(Lexer)
	lexer.Init()

	fi, err := os.Open(file_name)
	if err != nil {
		log.Fatal("open file error!")
	}
	defer fi.Close()

	reader := bufio.NewReader(fi)
	for {
		//fmt.Printf("%c", c)
		if !lexer.IsAnyOpen() {
			//log.Println("***again***")
			for _, r := range lexer.receivers {
				if !r.IsOpen() {
					r.Open()
				}
			}
		}

		c, _, err := reader.ReadRune()

		if err == io.EOF {
			err = nil
			break
		} else if err != nil {
			log.Fatal(err)
		}

		results := []bool{}

		for _, r := range lexer.receivers {
			if r.IsOpen() {
				isAccepted := r.Take(c)
				if isAccepted {
					results = append(results, isAccepted)
				}
			}
		}

		if len(results) == 0 {
			//c isn't accepted
			err := reader.UnreadRune()
			if err != nil {
				log.Fatal(err)
			}
		}
	}

	lexer.TailClean()

	for _, t := range lexer.tokens {
		fmt.Printf("%d-%s\n", t.ty, t.literal)
	}
}

func main() {
	read("example.scm")
}
