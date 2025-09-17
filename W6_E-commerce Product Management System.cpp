#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
using namespace std;

// ======================== INTERFACE: Discountable ==========================
// Interface trừu tượng: Bất cứ lớp nào kế thừa cũng phải cài hàm applyDiscount
class Discountable {
public:
    virtual double applyDiscount(double rate) = 0;
    virtual ~Discountable() = default;
};

// ======================== BASE CLASS: Product ==========================
class Product : public Discountable {
private:
    string id;
    string name;
    double price;
    int stock;

public:
    Product(const string& id, const string& name, double price, int stock)
        : id(id), name(name), price(price), stock(stock) {
    }

    virtual ~Product() = default;

    // Getter / Setter
    string getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    void setPrice(double p) { price = p; }
    void setStock(int s) { stock = s; }

    // Hàm hiển thị (đa hình)
    virtual void display() const {
        cout << "Product [" << id << "] " << name
            << " - $" << price << " | Stock: " << stock << endl;
    }

    // Cập nhật tồn kho
    virtual void updateStock(int quantity) {
        if (stock + quantity < 0)
            cout << "Not enough stock of " << name << endl;
        else
            stock += quantity;
    }

    // Áp dụng giảm giá (interface)
    double applyDiscount(double rate) override {
        if (rate < 0 || rate > 1) return price;
        return price * (1 - rate);
    }

    // So sánh theo ID (operator overload)
    bool operator==(const Product& other) const {
        return this->id == other.id;
    }
};

// ======================== DERIVED CLASSES ==========================
// Electronics kế thừa Product
class Electronics : public Product {
private:
    int warrantyMonths;

public:
    Electronics(const string& id, const string& name, double price, int stock, int warranty)
        : Product(id, name, price, stock), warrantyMonths(warranty) {
    }

    void display() const override {
        cout << "Electronics [" << getId() << "] " << getName()
            << " - $" << getPrice() << " | Stock: " << getStock()
            << " | Warranty: " << warrantyMonths << " months\n";
    }

    void updateStock(int quantity) override {
        cout << "(Electronics stock update includes fragile handling)\n";
        Product::updateStock(quantity);
    }
};

// Clothing kế thừa Product
class Clothing : public Product {
private:
    string size;

public:
    Clothing(const string& id, const string& name, double price, int stock, const string& size)
        : Product(id, name, price, stock), size(size) {
    }

    void display() const override {
        cout << "Clothing [" << getId() << "] " << getName()
            << " - $" << getPrice() << " | Stock: " << getStock()
            << " | Size: " << size << "\n";
    }
};

// ======================== TEMPLATE CLASS: InventoryList ==========================
// Template quản lý danh sách (tái sử dụng được)
template <typename T>
class InventoryList {
private:
    vector<T> items;

public:
    void add(const T& item) {
        items.push_back(item);
    }

    void remove(int index) {
        if (index >= 0 && index < (int)items.size())
            items.erase(items.begin() + index);
    }

    int size() const { return (int)items.size(); }

    T& get(int index) {
        if (index < 0 || index >= (int)items.size())
            throw out_of_range("Invalid index");
        return items[index];
    }

    const T& get(int index) const {
        if (index < 0 || index >= (int)items.size())
            throw out_of_range("Invalid index");
        return items[index];
    }

    void displayAll() const {
        for (auto& item : items)
            item->display();
    }
};

// ======================== CLASS: ShoppingCart ==========================
// Lưu danh sách sản phẩm + số lượng mua
struct CartItem {
    shared_ptr<Product> product;
    int quantity;
};

class ShoppingCart : public Discountable {
private:
    vector<CartItem> cartItems;
    double total = 0;

public:
    // Thêm sản phẩm vào giỏ: nếu trùng ID thì tăng số lượng
    ShoppingCart& operator+=(shared_ptr<Product> p) {
        if (p->getStock() <= 0) {
            cout << "Cannot add " << p->getName() << " (out of stock)\n";
            return *this;
        }

        bool found = false;
        for (auto& item : cartItems) {
            if (*item.product == *p) {
                item.quantity++;
                found = true;
                break;
            }
        }
        if (!found) {
            cartItems.push_back({ p, 1 });
        }

        total += p->getPrice();
        p->updateStock(-1);
        return *this;
    }

    void display() const {
        cout << "=== Cart Contents ===\n";
        for (auto& item : cartItems) {
            cout << item.quantity << "x ";
            item.product->display();
        }
        cout << "Total: $" << total << "\n";
    }

    double applyDiscount(double rate) override {
        if (rate < 0 || rate > 1) return total;
        total *= (1 - rate);
        return total;
    }
};

// ======================== CLASS: Order ==========================
class Order {
private:
    string orderId;
    ShoppingCart cart;

public:
    Order(const string& id, const ShoppingCart& c)
        : orderId(id), cart(c) {
    }

    void display() const {
        cout << "\n=== Order " << orderId << " ===\n";
        cart.display();
    }
};

// ======================== TESTING MAIN ==========================
int main() {
    // Tạo các sản phẩm
    auto p1 = make_shared<Product>("P01", "Book", 10.0, 5);
    auto p2 = make_shared<Electronics>("E01", "Laptop", 1200.0, 2, 24);
    auto p3 = make_shared<Clothing>("C01", "T-Shirt", 20.0, 3, "L");

    // Inventory sử dụng template
    InventoryList<shared_ptr<Product>> inventory;
    inventory.add(p1);
    inventory.add(p2);
    inventory.add(p3);

    cout << "\n--- INVENTORY ---\n";
    inventory.displayAll();

    // ShoppingCart demo
    ShoppingCart cart;
    cart += p1;
    cart += p2;
    cart += p3;
    cart += p2; // laptop lần 2 -> tăng quantity
    cart += p2; // hết stock -> báo lỗi

    cout << "\n--- CART BEFORE DISCOUNT ---\n";
    cart.display();

    // Discount
    cout << "\nApplying 10% discount...\n";
    cart.applyDiscount(0.1);
    cart.display();

    // So sánh
    cout << "\nCompare p1 and p2: "
        << ((*p1 == *p2) ? "same" : "different") << endl;

    // Order
    Order order1("O001", cart);
    order1.display();

    return 0;
}
