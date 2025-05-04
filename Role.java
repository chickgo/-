package com.klpbbs.model.enums;

import jakarta.persistence.*;
import java.util.Set;
import java.util.HashSet;

@Entity
public enum Role {
    ADMIN, USER
}