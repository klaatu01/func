function Sum (inp) returns res
begin
  res := 0;
  while Less(0,inp) loop
     res := Plus(res,inp);
     inp := Minus(inp,1);
  end loop; 
end function;

function Main () 
variables inp,res
begin
  read inp;
  res := Sum(inp);
  write res;
end function; 
