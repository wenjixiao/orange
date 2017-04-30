use std::fmt;
use std::collections::HashSet;
use std::io::prelude::*;
use std::io::BufReader;
use std::fs::File;

//-------------------------
#[derive(Clone)]
enum TokenType {
    LPAREN,RPAREN,QUOTE,NUMBER,STRING,SYMBOL,UNDEFINED
}

enum BucketType {
    SINGLE,NUMBER,STRING,SYMBOL,WHITESPACE
}

impl fmt::Display for BucketType {
    fn fmt(&self,f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            &BucketType::SINGLE => "SINGLE",
            &BucketType::WHITESPACE => "WHITESPACE",
            &BucketType::SYMBOL  => "SYMBOL",
            &BucketType::NUMBER => "NUMBER",
            &BucketType::STRING => "STRING",
        };
        write!(f,"{}",s)
    }
}

impl fmt::Display for TokenType {
    fn fmt(&self,f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            &TokenType::LPAREN => "LPAREN",
            &TokenType::RPAREN => "RPAREN",
            &TokenType::QUOTE  => "QUOTE",
            &TokenType::NUMBER => "NUMBER",
            &TokenType::STRING => "STRING",
            &TokenType::SYMBOL => "SYMBOL",
            &TokenType::UNDEFINED => "UNDEFINED",
        };
        write!(f,"{}",s)
    }
}

//-------------------------
#[derive(Clone)]
struct Token {
    ty: TokenType,
    literal: String,
}

impl fmt::Display for Token {
    fn fmt(&self,f: &mut fmt::Formatter) -> fmt::Result {
        write!(f,"type={},literal={}",self.ty,self.literal)
    }
}

//-------------------------
trait RuneReceiver {
    fn take(&mut self,container: &mut TokensContainer,c: char) -> bool;
    fn is_open(&self) -> bool;
    fn open(&mut self);
    fn close(&mut self,container: &mut TokensContainer);
}

trait TokenListener {
    fn heard(&mut self,t: Token);
}

//-------------------------
struct TokensContainer {
    tokens: Vec<Token>
}

impl TokenListener for TokensContainer {
    fn heard(&mut self,t: Token){
        self.tokens.push(t);
    }
}

impl TokensContainer {
    fn new() -> TokensContainer { 
        TokensContainer { 
            tokens: Vec::new()
        }
    }
    fn append_token(&mut self,t: Token){
        //println!("==={}===",t);
        self.tokens.push(t);
    }
    fn print_tokens(&self) {
        for t in &self.tokens {
            println!("{}",t)
        }
    }
}
//-------------------------
struct Bucket {
    bty: BucketType,
    ty: TokenType,
    is_open: bool,
    buf: String,
}

impl Bucket {
    fn new(bty: BucketType) -> Bucket {
        Bucket {
            bty: bty,
            ty: TokenType::UNDEFINED,
            is_open: false,
            buf: String::new(),
        }
    }
    fn append(&mut self,c: char) {
        self.buf.push(c)
    }
    fn take_number(&mut self,container: &mut TokensContainer,c: char) -> bool {
        //println!("^^^^{}^^^^",c);
        let digits = "0123456789";
        let h_chars = str_to_hashset(digits);
        if h_chars.contains(&c) {
            self.append(c);
            true
        } else {
            self.ty = TokenType::NUMBER;
            self.close(container);
            false
        }
    }
    fn take_symbol(&mut self,container: &mut TokensContainer,c: char) -> bool {
        let digits = "0123456789";
        let letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        let special = "!$%&*/:<=>?^_~";
        let special_sub = "+-.@";

        let h_digits: HashSet<char> = str_to_hashset(digits);
        let h_letters: HashSet<char> = str_to_hashset(letters);
        let h_special: HashSet<char> = str_to_hashset(special);
        let h_special_sub: HashSet<char> = str_to_hashset(special_sub);

        let h_initial: HashSet<char> = h_letters.union(&h_special).cloned().collect();
        let h_initial_digits: HashSet<char> = h_initial.union(&h_digits).cloned().collect();
        let h_subsequent: HashSet<char> = h_initial_digits.union(&h_special_sub).cloned().collect();

	    let mut is_peculiar = false;

        if self.buf.len() == 0 {
            if c == '+' || c == '-' {
                self.append(c);
                self.ty = TokenType::SYMBOL;
                self.close(container);
                true
            } else if c == '.' {
                self.append(c);
                is_peculiar = true;
                true
            } else {
                if h_initial.contains(&c) {
                    self.append(c);
                    true
                } else {
                    self.ty = TokenType::SYMBOL;
                    self.close(container);
                    false
                }
            }
        } else {
            if is_peculiar {
                if c == '.' && self.buf.len() < 3 {
                    self.append(c);
                    true
                } else {
                    self.ty = TokenType::SYMBOL;
                    self.close(container);
                    false
                }

            } else {
                if h_subsequent.contains(&c) {
                    self.append(c);
                    true
                } else {
                    self.ty = TokenType::SYMBOL;
                    self.close(container);
                    false
                }
            }
        }
    }
    fn take_string(&mut self,container: &mut TokensContainer,c: char) -> bool {
        if c == '"' {
            if self.buf.len() > 0 {
                self.append(c);
                self.ty = TokenType::STRING;
                self.close(container);
                true
            } else {
                self.append(c);
                true
            }
        } else {
            if self.buf.len() > 0 {
                self.append(c);
                true
            } else {
                self.ty = TokenType::STRING;
                self.close(container);
                false
            }
        }
    }
    fn take_single(&mut self,container: &mut TokensContainer,c: char) -> bool {
        //println!("@@{}",c);
        match c {
            '('   => {
                self.ty = TokenType::LPAREN;
                self.append(c);
                self.close(container);
                true 
            }
            ')'  => {
                self.ty = TokenType::RPAREN;
                self.append(c);
                self.close(container);
                true 
            }
            '\'' => {
                self.ty = TokenType::QUOTE;
                self.append(c);
                self.close(container);
                true
            }
            _ => {
                self.close(container);
                false 
            }
        }
    }
    fn take_whitespace(&mut self,container: &mut TokensContainer,c: char) -> bool {
        //println!("*****{}*****",c);
        let ws = " \n\t\r\\v\\f";
        let h_chars = str_to_hashset(ws);
        if h_chars.contains(&c) {
            true
        }else{
            self.close(container);
            false
        }
    }
}

fn str_to_hashset(s: &str) -> HashSet<char> {
    let mut hash_chars: HashSet<char> = HashSet::new();
    for c in s.chars() {
        hash_chars.insert(c);
    }
    hash_chars
}

impl RuneReceiver for Bucket {
    fn take(&mut self,container: &mut TokensContainer,c: char) -> bool {
        match self.bty {
            BucketType::NUMBER => self.take_number(container,c),
            BucketType::WHITESPACE => self.take_whitespace(container,c),
            BucketType::STRING => self.take_string(container,c),
            BucketType::SINGLE => self.take_single(container,c),
            BucketType::SYMBOL => self.take_symbol(container,c),
        }
    }
    fn is_open(&self) -> bool { self.is_open }
    fn open(&mut self) { 
        if !self.is_open {
            self.is_open = true
        }
    }
    fn close(&mut self,container: &mut TokensContainer) {
        if self.buf.len() > 0 {
            let t = Token{ty: self.ty.clone(),literal: self.buf.clone()};
            container.append_token(t);
        }
        self.buf.clear();
        self.is_open = false;
    }
}
//-------------------------
fn main() {
    let b1 = Bucket::new(BucketType::NUMBER);
    let b2 = Bucket::new(BucketType::WHITESPACE);
    let b3 = Bucket::new(BucketType::SINGLE);
    let b4 = Bucket::new(BucketType::STRING);
    let b5 = Bucket::new(BucketType::SYMBOL);

    let mut container = TokensContainer::new();
    let mut buckets = vec![b1,b2,b3,b4,b5];

    let f = File::open("example.scm").expect("***open file error***");
    let buf_reader = BufReader::new(f);

    let mut myc: char = '0';
    let mut accepted_count = 1;
    let mut byte_iter = buf_reader.bytes();

    loop {
        //println!("=============================================");
        if accepted_count > 0 {
            match byte_iter.next() {
                Some(n) => { myc = n.unwrap() as char; },
                None => { break },
            };
            accepted_count = 0;
        }
        //println!("--{}--",myc);
        let is_all_closed: bool = buckets.iter().all(|b| b.is_open() == false);
        if is_all_closed {
            //println!("^^all closed^^");
            for b in &mut buckets {
                b.open();
            }
        }
        for b in &mut buckets {
            if b.is_open() {
                let bb = b.take(&mut container,myc);
                //println!("{}<<{}={}",b.bty,myc,bb);
                if bb { accepted_count+=1; }
            }
        }
    }
    // after loop,we need clear the buckets finally
    for b in &mut buckets {
        if b.is_open() {
            b.close(&mut container);
        }
    }
    println!("*************i am here*************");
    container.print_tokens();
}
