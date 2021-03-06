/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include "../eosio.token/eosio.token.cpp"
#include <cmath>

#define EOS_SYMBOL S(4, EOS)
#define HPY_SYMBOL S(4, HPY2)
#define TOKEN_CONTRACT N(eosio.token)

using std::string;
using eosio::symbol_name;
using eosio::asset;
using eosio::symbol_type;
using eosio::contract;
using eosio::permission_level;
using eosio::action;

typedef double real_type;

class tradeableToken : public eosio::token {
    public:
        tradeableToken(account_name self) : token(self), _market(_self, _self) {}

        void init(asset eos);
        void test(const account_name account, asset eos);
        void buy(const account_name account, asset eos);
        void sell(const account_name account, asset hpy);
        void onTransfer(account_name from, account_name to, asset eos, std::string memo);

        uint64_t get_deposit() const{
            auto market_itr = _market.begin();
            return market_itr->deposit.balance.amount - init_quote_balance;  
        }

        // @abi table market i64
        struct exchange_state {
            uint64_t id = 0;
            asset supply;
            struct connector {
                asset balance;  
                double weight = .0005;
                EOSLIB_SERIALIZE(connector, (balance)(weight))
                real_type price() {
                    return balance.amount / 100000; 
                }
            } deposit;

            uint64_t primary_key() const { return id; }

            /*
            asset convert_to_exchange(connector &c, asset in) {
                real_type R(supply.amount);
                real_type C(c.balance.amount + in.amount);
                real_type F(c.weight);
                real_type T(in.amount);
                real_type ONE(1.0);

                real_type E = -R * (ONE - pow(ONE + T / C, F));
                int64_t issued = int64_t(E);

                supply.amount += issued;
                c.balance.amount += in.amount;

                return asset(issued, supply.symbol);
            }
            */
            asset convert_to_exchange(connector &c, asset in) {
                real_type a = c.price();
                real_type out = (-a + pow(a*a + 2*c.weight*in.amount, 0.5)) / (c.weight);
                return asset(out, c.balance.symbol);
            }

            asset convert_from_exchange(connector &c, asset in) {
                real_type a = c.price();
                real_type out = (-a + pow(a*a - 2*c.weight*in.amount, 0.5)) / (-c.weight);
                return asset(out, c.balance.symbol);    
                /*real_type R(supply.amount - in.amount);
                real_type C(c.balance.amount);
                real_type F(1.0 / c.weight);
                real_type E(in.amount);
                real_type ONE(1.0);

                real_type T = C * (pow(ONE + E / R, F) - ONE);
                int64_t out = int64_t(T);

                supply.amount -= in.amount;
                c.balance.amount -= out;

                return asset(out, c.balance.symbol);*/
            }            

            /*asset convert_from_exchange(connector &c, asset in) {
                real_type R(supply.amount - in.amount);
                real_type C(c.balance.amount);
                real_type F(1.0 / c.weight);
                real_type E(in.amount);
                real_type ONE(1.0);

                real_type T = C * (pow(ONE + E / R, F) - ONE);
                int64_t out = int64_t(T);

                supply.amount -= in.amount;
                c.balance.amount -= out;

                return asset(out, c.balance.symbol);
            }
            */

            asset convert(asset from, symbol_type to) {
                if (from.symbol == EOS_SYMBOL && to == HPY_SYMBOL) {
                    return convert_to_exchange(deposit, from);
                } else if (from.symbol == HPY_SYMBOL && to == EOS_SYMBOL) {
                    return convert_from_exchange(deposit, from);
                } else {
                    eosio_assert(false, "illegal convertion.");
                    return from;
                }
            }

            EOSLIB_SERIALIZE(exchange_state, (supply)(deposit))
        };

        typedef eosio::multi_index<N(market), exchange_state> market;
        market _market;

        // tradeableToken
        const uint64_t init_quote_balance = 1 * 10000 * 10000ll; // 初始保证金 1 万 EOS。;  
};