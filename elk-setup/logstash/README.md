# Logstash config forklating

## Input
Siger til logstash det kommer ind som http.
host 0.0.0.0 gør der lyttes på alt der kommer ind.
port 31311 er porten der sendes data i mod. Hvis du bare smider data mod logstash.alexanderrasmussen.dk så skal mit interne setup når sikre at denne port bliver ramt.

## Filter
Det er her at der skal laves ændringer. Vi siger det kommer ind som CSV da en komma separeret streng ligner en CSV fil.

Lad os sige du ønsker at tilføje IP til systemet, så skal du i colums tilføje navnet IP-address på den plads som passer til den plads det bliver sendt ind på i. Derefter skal du fortælle hvordan IP mappes, som det er til en Int, String eller noget andet [Logstash data typer](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html).  

Det kunne komme til at se sådan her ud

```
filter {
  csv {
    columns => ["temperature", "light", "IP-address"]
    separator => ","
    convert => {
     "temperature" => "integer"
     "light" => "integer"
     "IP-address" => "IP"
    }
  }
}
```

## Output
Det eneste interressant her er måske index som er det vi kan vælge når vi søger, men det tænker jeg ikke vi får brug for.