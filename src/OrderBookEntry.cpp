#include "OrderBookEntry.h"
#include <iostream>
#include <string>

OrderBookEntry::OrderBookEntry( double _price, 
                        double _amount, 
                        std::string _timestamp, 
                        std::string _product, 
                        OrderBookType _orderType, 
                        std::string _username)
: price(_price), 
  amount(_amount), 
  timestamp(_timestamp),
  product(_product), 
  orderType(_orderType), 
  username(_username)
{
  
    
}

OrderBookType OrderBookEntry::stringToOrderBookType(std::string s)
{
  if (s == "ask")     return OrderBookType::ask;
  if (s == "bid")     return OrderBookType::bid;
  if (s == "asksale") return OrderBookType::asksale;
  if (s == "bidsale") return OrderBookType::bidsale;
  
  return OrderBookType::unknown;
}

std::string OrderBookEntry::orderBookTypeToString(OrderBookType orderType)
{
  if (orderType == OrderBookType::ask)     return "ask";
  if (orderType == OrderBookType::bid)     return "bid";
  if (orderType == OrderBookType::asksale) return "asksale";
  if (orderType == OrderBookType::bidsale) return "bidsale";
  
  return "";
}

std::string OrderBookEntry::toString()
{
  // std::cout << product << ", "<< orderBookTypeToString(orderType) <<", "<<timestamp<<", "<<price<<", "<<amount<<std::endl;
  std::string conc = product + ", " + orderBookTypeToString(orderType) + ", " + timestamp + ", " + std::to_string(price)+ ", " + std::to_string(amount);
  return conc;
}